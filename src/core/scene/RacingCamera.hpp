#pragma once

#include "BaseCamera.hpp"

// Follow camera tuned for racing: spring-smoothed position + speed-reactive FOV.
class RacingCamera : public BaseCamera {
public:
    bool isSpringEnabled = true;    // Apply spring forces for smooth follow
    bool isDragEnabled = true;      // Apply spring drag (damping)
    bool isFovEnabled = true;       // Adjust FOV based on speed
    bool isShakeEnabled = true;     // Add shake based on jerk (change in acceleration)
    bool isShakeLowToHigh = false;  // False does the opposite: low intensity = more shake frequency

    explicit RacingCamera() = default;

    // Store the latest tracked vehicle state
    void updateTarget(glm::vec3 targetPos, glm::vec3 targetForward, glm::vec3 targetVelocity);

    // Advance spring and FOV once per render frame
    void update(float dt);

    glm::mat4 viewMatrix() override;
    std::string toString() const override;

private:
    glm::vec3 mTargetPos{};
    glm::vec3 mTargetForward{ 0.0f, 0.0f, 1.0f };
    glm::vec3 mTargetVelocity{};
    float mTargetSpeedMs = 0.0f;
    bool mHasTarget = false;
    bool mInitialized = false;

    glm::vec3 mLookAt{};        // Point the camera looks at (target position + look-ahead)
    glm::vec3 mSpringOffset{};  // Smoothed offset from the target
    glm::vec3 mSpringVel{};     // Velocity of the offset spring

    float mArmLength = 6.4f;    // Ideal distance behind the target
    float mArmHeight = 2.2f;    // Ideal height above the target
    float mStiffness = 20.0f;   // Spring strength (ks); higher = snappier follow

    // Damping ratio: zeta = kd / (2*sqrt(ks)).
    // zeta = 1.0 is critically damped (no oscillation).
    // zeta < 1 = underdamped (slight bounce).
    // zeta > 1 = overdamped (sluggish).
    float mDampingRatio = 0.85f;
    float mLookAheadDist = 5.f;  // To anticipate frame motion and turns

    float mCurrentFovDeg{};         // Current FOV in degrees used by the rate limiter
    float mMinFovDeg = 55.f;        // Base field of view in degrees
    float mMaxFovDeg = 130.f;       // Upper FOV bound at high speed
    float mFovSpeedAtMax = 35.f;    // Speed (m/s) that maps to max FOV
    float mFovRiseLambda = 3.75f;   // Speed increase response for FOV
    float mFovFallLambda = 0.75f;   // Speed decrease response for FOV
    float mMaxFovStepFrame = 2.0f;  // Hard cap on FOV change per frame (degrees)
    float mFovFilteredSpeed{};      // Single source of truth for filtered speed

    // Jerk-driven shake (jerk = d(acceleration)/dt).
    float mShakePosAmp = 0.06f;         // Max positional shake in meters
    float mShakeJerkAtMax = 300.0f;     // m/s^3 that maps to full shake intensity
    float mShakeMinFreqHz = 5.0f;       // Min shake frequency at low speed
    float mShakeMaxFreqHz = 12.0f;      // Max shake frequency at high speed
    float mShakeLambda = 15.0f;         // Response speed for shake intensity
    float mShakeDecay = 5.0f;           // Decay rate for shake intensity (higher = faster decay)

    // Shake smoothing and thresholds
    float mShakeDeadzone = 0.1f;        // Suppress residual intensity
    float mShakeJerkDeadband = 50.0f;   // Ignore low-amplitude jerk noise
    float mShakeMinSpeedMs = 0.5f;      // Disable shake near idle
    float mSpeedSmoothingTime = 0.12f;  // How long to smooth speed for shake (seconds)
    float mSpeedBaseline = 0.15f;       // Baseline shake intensity at max speed

    // Tracked state for shake calculations
    glm::vec3 mShakePosOffset{};
    float mShakeIntensity = 0.0f;
    float mShakeTime = 0.0f;
    float mSmoothSpeed = 0.0f;
    float mPrevSpeedMs = 0.0f;
    float mPrevAccelMs2 = 0.0f;

    void init(glm::vec3 const& idealOffset);
    void updateSpring(float dt, glm::vec3 const& idealOffset);
    void updateFov(float dt);
    void updateShake(float dt, glm::vec3 const& forward, glm::vec3 const& right);
    float targetFovDegrees() const;
};
