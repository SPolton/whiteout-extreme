#pragma once

#include "BaseCamera.hpp"

// Follow camera tuned for racing: spring-smoothed position + speed-reactive FOV.
class RacingCamera : public BaseCamera {
public:
    bool isSpringEnabled = true; // Apply spring forces for smooth follow
    bool isDragEnabled = true;   // Apply spring drag (damping)
    bool isFovEnabled = true;    // Adjust FOV based on speed

    explicit RacingCamera() = default;

    // Store the latest tracked vehicle state
    void updateTarget(glm::vec3 targetPos, glm::vec3 targetForward, float speedMs);

    // Advance spring and FOV once per render frame
    void update(float dt);

    glm::mat4 viewMatrix() override;
    glm::vec3 position() override;
    std::string toString() const override;

private:
    glm::vec3 mTargetPos{};
    glm::vec3 mTargetForward{ 0.0f, 0.0f, 1.0f };
    float mTargetSpeedMs = 0.0f;
    bool mHasTarget = false;
    bool mInitialized = false;

    glm::vec3 mLookAt{};        // Point the camera looks at (target position + look-ahead)
    glm::vec3 mSpringPos{};     // Final camera world position
    glm::vec3 mSpringOffset{};  // Smoothed offset from the target
    glm::vec3 mSpringVel{};     // Velocity of the offset spring

    float mArmLength = 7.0f;    // Ideal distance behind the target
    float mArmHeight = 2.0f;    // Ideal height above the target
    float mStiffness = 30.0f;   // Spring strength (ks); higher = snappier follow

    // Damping ratio: zeta = kd / (2*sqrt(ks)).
    // zeta = 1.0 is critically damped (no oscillation).
    // zeta < 1 = underdamped (slight bounce).
    // zeta > 1 = overdamped (sluggish).
    float mDampingRatio = 0.9f;
    float mLookAheadDist = 5.f;  // To anticipate frame motion and turns

    float mCurrentFovDeg{};      // Current FOV in degrees used by the rate limiter
    float mMinFovDeg = 50.f;     // Base field of view in degrees
    float mMaxFovDeg = 120.f;    // Upper FOV bound at high speed
    float mFovSpeedAtMax = 25.f; // Speed (m/s) that maps to max FOV
    float mFovRiseLambda = 2.0f; // Speed increase response for FOV
    float mFovFallLambda = 0.5f; // Speed decrease response for FOV
    float mMaxFovStepFrame = 2.0f; // Hard cap on FOV change per frame (degrees)
    float mFovFilteredSpeed{};   // Single source of truth for filtered speed

    void init(glm::vec3 const& idealOffset);
    void updateFov(float dt);
    float targetFovDegrees() const;
};
