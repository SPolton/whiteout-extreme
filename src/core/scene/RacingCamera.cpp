#include "RacingCamera.hpp"

#include <ext/matrix_transform.hpp>
#include <fmt/format.h>

// The core math is equivalent to this article for a force-at-rest spring system
// https://phramebuffer.wordpress.com/third-person-camera-with-spring-system/

void RacingCamera::init(glm::vec3 const& idealOffset)
{
    if (mInitialized) return;
    mInitialized = true;

    mSpringOffset = idealOffset;
    mSpringVel = glm::vec3(0.0f);
    mPosition = mTargetPos + mSpringOffset;

    mTargetSpeedMs = glm::length(mTargetVelocity);
    mFovFilteredSpeed = glm::max(mTargetSpeedMs, 0.0f);
    mSmoothSpeed = mFovFilteredSpeed;
    mPrevSpeedMs = mSmoothSpeed;
    mCurrentFovDeg = targetFovDegrees();
    fov(mCurrentFovDeg);
}

void RacingCamera::updateTarget(glm::vec3 targetPos, glm::vec3 targetForward, glm::vec3 targetVelocity)
{
    mTargetPos = targetPos;
    mTargetForward = targetForward;
    mTargetVelocity = targetVelocity;
    mTargetSpeedMs = glm::length(targetVelocity);
    mHasTarget = true;
}

void RacingCamera::update(float dt)
{
    if (!mHasTarget) return;

    // Use a safe fallback if input forward is near-zero to avoid NaNs during normalize().
    glm::vec3 forward = glm::length(mTargetForward) > 0.0001f
        ? glm::normalize(mTargetForward)
        : glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 const right = glm::normalize(glm::cross(forward, mUp));

    // Desired chase offset relative to the current vehicle position.
    // Smoothing the offset instead of the camera position avoids visible translation lag.
    glm::vec3 const idealOffset = -forward * mArmLength + mUp * mArmHeight;

    init(idealOffset);
    updateSpring(dt, idealOffset);
    updateFov(dt);
    updateShake(dt, forward, right);

    glm::vec3 const springPos = mTargetPos + mSpringOffset;

    // Base follow camera, add same shake to eye + target (rigid, not orbital).
    // Look slightly ahead to anticipate turns.
    mLookAt = mTargetPos + forward * mLookAheadDist + mShakePosOffset;
    mPosition = springPos + mShakePosOffset;
}

void RacingCamera::updateSpring(float dt, glm::vec3 const& idealOffset)
{
    if (!isSpringEnabled) {
        // Drag disabled means no spring simulation: lock to ideal offset immediately.
        mSpringOffset = idealOffset;
        mSpringVel = glm::vec3(0.0f);
    }
    else if (dt > 0.0f) {
        // Convert damping ratio to a damping coefficient so tuning remains stable as ks changes.
        // kd = zeta * 2*sqrt(ks) where zeta=1 is critical damping.
        float const kd = isDragEnabled ? (mDampingRatio * 2.0f * glm::sqrt(mStiffness)) : 0.0f;

        // Spring-damper force (unit mass): F = ks*(x_target - x) - kd*v.
        // Treat mass as 1, so acceleration = force.
        glm::vec3 const force = mStiffness * (idealOffset - mSpringOffset) - kd * mSpringVel;

        // Semi-implicit Euler integration: update velocity, then position.
        mSpringVel += force * dt;
        mSpringOffset += mSpringVel * dt;
    }
}

void RacingCamera::updateFov(float dt)
{
    if (!isFovEnabled) return;

    if (dt <= 0.0f) {
        mCurrentFovDeg = targetFovDegrees();
        fov(mCurrentFovDeg);
        return;
    }

    // Exponential (1st-order IIR) low-pass filter on speed, framerate-independent:
    //   alpha = 1 - e^(-lambda * dt)
    // alpha->0 means very slow tracking; alpha->1 means instant.
    // Separate lambdas for rises vs falls let acceleration widen FOV faster
    // than deceleration narrows it, which feels more natural.
    float const targetSpeed = glm::max(mTargetSpeedMs, 0.0f);
    float const lambda = targetSpeed >= mFovFilteredSpeed ? mFovRiseLambda : mFovFallLambda;
    float const speedAlpha = 1.0f - std::exp(-lambda * dt);
    mFovFilteredSpeed = glm::mix(mFovFilteredSpeed, targetSpeed, speedAlpha);

    // Clamp the per-frame FOV delta to mMaxFovStepFrame degrees.
    // This is a hard ceiling: no matter how large the filtered-speed jump,
    // the visible FOV can never change by more than that many degrees in one frame.
    float const targetFovDeg = targetFovDegrees();
    float const delta = glm::clamp(targetFovDeg - mCurrentFovDeg, -mMaxFovStepFrame, mMaxFovStepFrame);
    mCurrentFovDeg += delta;

    fov(mCurrentFovDeg);
}

float RacingCamera::targetFovDegrees() const
{
    // Normalise filtered speed into [0, 1] where 1 = mFovSpeedAtMax.
    float speedNorm = glm::clamp(mFovFilteredSpeed / mFovSpeedAtMax, 0.0f, 1.0f);

    // Smoothstep: f(t) = t^2 (3 - 2t)
    // Produces an S-curve with zero derivative at t=0 and t=1, so the FOV
    // eases in gently at low speed and plateaus softly near mFovSpeedAtMax.
    speedNorm = speedNorm * speedNorm * (3.0f - 2.0f * speedNorm);

    // Linear blend: FOV = base + (max - base) * smoothstep(speed)
    return glm::mix(mMinFovDeg, mMaxFovDeg, speedNorm);
}

void RacingCamera::updateShake(float dt, glm::vec3 const& forward, glm::vec3 const& right)
{
    (void)forward;
    float const targetSpeed = glm::length(mTargetVelocity);

    if (!isShakeEnabled || dt <= 0.0f || targetSpeed < mShakeMinSpeedMs) {
        mShakePosOffset = glm::vec3(0.0f);
        mShakeIntensity = 0.0f;
        mShakeTime = 0.0f;
        mSmoothSpeed = targetSpeed;
        mPrevSpeedMs = targetSpeed;
        mPrevAccelMs2 = 0.0f;
        return;
    }

    // Use change in acceleration (jerk) to drive a procedural shake effect.
    // Smooth speed to suppress frame-to-frame jitter before differentiation.
    float const speedLambda = 1.0f / mSpeedSmoothingTime;
    float const speedAlpha  = 1.0f - exp(-speedLambda * dt);
    mSmoothSpeed = glm::mix(mSmoothSpeed, targetSpeed, speedAlpha);

    // Derivatives from smoothed speed.
    float const accelMs2 = (mSmoothSpeed - mPrevSpeedMs) / dt;
    float const jerkMs3 = std::abs((accelMs2 - mPrevAccelMs2) / dt);

    // Threshold and normalize so only meaningful jolts contribute.
    float const jerkEffective = glm::max(jerkMs3 - mShakeJerkDeadband, 0.0f);
    float const jerkNorm = glm::clamp(jerkEffective / mShakeJerkAtMax, 0.0f, 1.0f);
    float const jerkResponse = std::sqrt(jerkNorm);

    // Impulse-style response: decay continuously, but take immediate peaks.
    mShakeIntensity *= std::exp(-mShakeDecay * dt);
    mShakeIntensity = glm::max(mShakeIntensity, jerkResponse);
    if (mShakeIntensity < mShakeDeadzone) {
        mShakeIntensity = 0.0f;
    }

    // High-speed baseline shake (small vibration at high speed)
    float const speedNorm = glm::clamp(targetSpeed / mFovSpeedAtMax, 0.0f, 1.0f); // Reuse FOV speed normalization
    float const baseVibe = speedNorm * mSpeedBaseline;
    mShakeIntensity = glm::max(mShakeIntensity, baseVibe);

    mShakeTime += dt;
    mPrevSpeedMs = mSmoothSpeed;
    mPrevAccelMs2 = accelMs2;

    // Frequency modulation that adapts to intensity to feel more natural
    float freq;
    if (isShakeLowToHigh)
        freq = glm::mix(mShakeMinFreqHz, mShakeMaxFreqHz, mShakeIntensity); // low to high
    else
        freq = glm::mix(mShakeMaxFreqHz, mShakeMinFreqHz, mShakeIntensity); // high to low

    float const w = 2.0f * glm::pi<float>() * freq;
    float const t = mShakeTime;

    // Use a sum of sines for better shake pattern.
    float const sx =
        0.6f * std::sin(w * t) +
        0.3f * std::sin(2.3f * w * t + 0.7f) +
        0.1f * std::sin(4.7f * w * t + 2.1f);

    float const sy =
        0.6f * std::sin(1.2f * w * t + 1.1f) +
        0.3f * std::sin(2.1f * w * t + 0.3f) +
        0.1f * std::sin(3.9f * w * t + 2.7f);

    // Vertical shake is usually more noticeable, so scale it down a bit.
    float const verticalAmp = glm::mix(0.5f, 0.7f, mShakeIntensity);
    mShakePosOffset =
        right * (sx * mShakePosAmp * mShakeIntensity)
        + mUp * (sy * mShakePosAmp * verticalAmp * mShakeIntensity);
}

glm::mat4 RacingCamera::viewMatrix()
{
    // The lookAt target is the vehicle position plus a look-ahead in the forward direction.
    return glm::lookAt(position(), mLookAt, mUp);
}

std::string RacingCamera::toString() const
{
    return fmt::format(
        "Racing Camera\n"
        "Pos: ({:.1f}, {:.1f}, {:.1f})\n"
        "LookAt: ({:.1f}, {:.1f}, {:.1f})\n"
        "Damping ratio: {:.2f}  Arm: {:.1f}m / {:.1f}m\n"
        "FOV: {:.1f}  Speed: {:.1f} m/s ({:.1f} m/s)\n"
        "Shake: {:.1f}  Acceleration: {:.1f}",
        mPosition.x, mPosition.y, mPosition.z,
        mLookAt.x, mLookAt.y, mLookAt.z,
        mDampingRatio, mArmLength, mArmHeight,
        glm::degrees(mFov), mTargetSpeedMs, mFovFilteredSpeed,
        mShakeIntensity, mPrevAccelMs2
    );
}
