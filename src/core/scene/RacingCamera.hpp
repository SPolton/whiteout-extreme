#pragma once

#include "BaseCamera.hpp"

// Follow camera tuned for racing: spring-smoothed position + speed-reactive FOV.
class RacingCamera : public BaseCamera {
public:
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
    float mSmoothedSpeed = 0.f;  // Low-pass filtered speed for smooth FOV changes
    float mLookAheadDist = 5.f;  // To anticipate frame motion and turns

    float mBaseFovDeg = 60.f;    // Base field of view in degrees
    float mFovGain = 0.0f;       // How much FOV widens with speed (deg per m/s)
    float mFovFilteredSpeed{};   // Low-pass filtered speed for FOV calculation

    void init(glm::vec3 const& idealOffset);
};
