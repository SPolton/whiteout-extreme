#include "RacingCamera.hpp"

#include "utils/logger.h"

// The core math is equivalent to this article for a force-at-rest spring system
// https://phramebuffer.wordpress.com/third-person-camera-with-spring-system/

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

    if (!mInitialized) {
        mSpringOffset = idealOffset;
        mSpringVel = glm::vec3(0.0f);
        mSmoothedSpeed = glm::max(mTargetSpeedMs, 0.0f);
        mSpringPos = mTargetPos + mSpringOffset;
        mInitialized = true;
    }

    if (dt > 0.0f) {
        // Convert damping ratio to a damping coefficient so tuning remains stable as ks changes.
        // kd = zeta * 2*sqrt(ks) where zeta=1 is critical damping.
        float const kd = mDampingRatio * 2.0f * glm::sqrt(mStiffness);

        // Spring-damper force (unit mass): F = ks*(x_target - x) - kd*v.
        // We treat mass as 1, so acceleration = force.
        glm::vec3 const force = mStiffness * (idealOffset - mSpringOffset) - kd * mSpringVel;

        // Semi-implicit Euler integration: update velocity, then position.
        mSpringVel += force * dt;
        mSpringOffset += mSpringVel * dt;

        // Exponential smoothing that is framerate independent.
        // alpha = 1-exp(-lambda*dt) approximates a 1st-order low-pass filter.
        float const alpha = 1.0f - std::exp(-8.0f * dt);
        mSmoothedSpeed = glm::mix(mSmoothedSpeed, glm::max(mTargetSpeedMs, 0.0f), alpha);
    }

    // Speed-reactive FOV with hard comfort clamps.
    float const targetFovDeg = glm::clamp(mBaseFovDeg + mFovGain * mSmoothedSpeed, 45.0f, 110.0f);
    fov(targetFovDeg);

    mSpringPos = mTargetPos + mSpringOffset;

    // Look a bit ahead of the vehicle so framing anticipates turns.
    mLookAt = mTargetPos + forward * mLookAheadDist;
    mPosition = mSpringPos;
}

glm::mat4 RacingCamera::viewMatrix()
{
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
        "FOV: {:.1f}  Damping ratio: {:.2f}",
        mSpringPos.x, mSpringPos.y, mSpringPos.z,
        mLookAt.x, mLookAt.y, mLookAt.z,
        glm::degrees(mFov), mDampingRatio
    );
}
