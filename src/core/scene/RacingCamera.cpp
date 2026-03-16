#include "RacingCamera.hpp"

#include "utils/logger.h"

// The core math is equivalent to this article for a force-at-rest spring system
// https://phramebuffer.wordpress.com/third-person-camera-with-spring-system/

void RacingCamera::init(glm::vec3 const& idealOffset)
{
    if (mInitialized) return;
    mInitialized = true;

    mSpringOffset = idealOffset;
    mSpringVel = glm::vec3(0.0f);
    mSpringPos = mTargetPos + mSpringOffset;

    mFovFilteredSpeed = glm::max(mTargetSpeedMs, 0.0f);
    mCurrentFovDeg = targetFovDegrees();
    fov(mCurrentFovDeg);
}

void RacingCamera::updateTarget(glm::vec3 targetPos, glm::vec3 targetForward, float speedMs)
{
    mTargetPos = targetPos;
    mTargetForward = targetForward;
    mTargetSpeedMs = speedMs;
    mHasTarget = true;
}

void RacingCamera::update(float dt)
{
    if (!mHasTarget) return;

    // Use a safe fallback if input forward is near-zero to avoid NaNs during normalize().
    glm::vec3 forward = glm::length(mTargetForward) > 0.0001f
        ? glm::normalize(mTargetForward)
        : glm::vec3(0.0f, 0.0f, 1.0f);

    // Desired chase offset relative to the current vehicle position.
    // Smoothing the offset instead of the camera position avoids visible translation lag.
    glm::vec3 const idealOffset = -forward * mArmLength + mUp * mArmHeight;

    init(idealOffset);

    if (dt > 0.0f) {
        // Convert damping ratio to a damping coefficient so tuning remains stable as ks changes.
        // kd = zeta * 2*sqrt(ks) where zeta=1 is critical damping.
        float const kd = mDampingRatio * 2.0f * glm::sqrt(mStiffness);

        // Spring-damper force (unit mass): F = ks*(x_target - x) - kd*v.
        // Treat mass as 1, so acceleration = force.
        glm::vec3 const force = mStiffness * (idealOffset - mSpringOffset) - kd * mSpringVel;

        // Semi-implicit Euler integration: update velocity, then position.
        mSpringVel += force * dt;
        mSpringOffset += mSpringVel * dt;
    }

    updateFov(dt);

    mSpringPos = mTargetPos + mSpringOffset;

    // Look a bit ahead of the vehicle so framing anticipates turns.
    mLookAt = mTargetPos + forward * mLookAheadDist;
    mPosition = mSpringPos;
}

void RacingCamera::updateFov(float dt)
{
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
    float const speedAlpha = 1.0f - std::exp(-glm::max(lambda, 0.01f) * dt);
    mFovFilteredSpeed = glm::mix(mFovFilteredSpeed, targetSpeed, speedAlpha);

    // Clamp the per-frame FOV delta to mMaxFovStepFrame degrees.
    // This is a hard ceiling: no matter how large the filtered-speed jump,
    // the visible FOV can never change by more than that many degrees in one frame.
    float const targetFovDeg = targetFovDegrees();
    float const maxStep = glm::max(mMaxFovStepFrame, 0.01f);
    float const delta = glm::clamp(targetFovDeg - mCurrentFovDeg, -maxStep, maxStep);
    mCurrentFovDeg += delta;

    fov(mCurrentFovDeg);
}

float RacingCamera::targetFovDegrees() const
{
    // Normalise filtered speed into [0, 1] where 1 = mFovSpeedAtMax.
    float speedNorm = glm::clamp(mFovFilteredSpeed / glm::max(mFovSpeedAtMax, 0.001f), 0.0f, 1.0f);

    // Smoothstep: f(t) = t^2 (3 - 2t)
    // Produces an S-curve with zero derivative at t=0 and t=1, so the FOV
    // eases in gently at low speed and plateaus softly near mFovSpeedAtMax.
    speedNorm = speedNorm * speedNorm * (3.0f - 2.0f * speedNorm);

    // Linear blend: FOV = base + (max - base) * smoothstep(speed)
    return glm::mix(mMinFovDeg, mMaxFovDeg, speedNorm);
}

glm::mat4 RacingCamera::viewMatrix()
{
    // The lookAt target is the vehicle position plus a look-ahead in the forward direction.
    return glm::lookAt(mSpringPos, mLookAt, mUp);
}

glm::vec3 RacingCamera::position()
{
    return mSpringPos;
}

std::string RacingCamera::toString() const
{
    return fmt::format(
        "Racing Camera\n"
        "Pos: ({:.1f}, {:.1f}, {:.1f})\n"
        "LookAt: ({:.1f}, {:.1f}, {:.1f})\n"
        "FOV: {:.1f}  Speed: {:.1f} m/s ({:.1f} m/s)\n"
        "Damping ratio: {:.2f}  Arm: {:.1f}m / {:.1f}m",
        mSpringPos.x, mSpringPos.y, mSpringPos.z,
        mLookAt.x, mLookAt.y, mLookAt.z,
        glm::degrees(mFov), mTargetSpeedMs, mFovFilteredSpeed,
        mDampingRatio, mArmLength, mArmHeight
    );
}
