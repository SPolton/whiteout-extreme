#pragma once

#include "utils/math.hpp"

#include "BaseCamera.hpp"
#include "Transform.hpp"

class TurnTableCamera : public BaseCamera {
public:

    struct Params
    {
        float defaultRadius = 0.0f;
        float defaultTheta = 0.0f;
        float defaultPhi = 0.0f;

        float defaultDistance = 5.0f;
        float minDistance = 1.0f;
        float maxDistance = 20.0f;
    };

    // Angles are in radians. Dummy target is set at (0,0,0)
    explicit TurnTableCamera();
    explicit TurnTableCamera(Params const& params);

    // Angles are in radians
    explicit TurnTableCamera(SceneTransform& target);
    explicit TurnTableCamera(SceneTransform& target, Params const& params);

    void target(SceneTransform& target);

    void adjustTheta(float deltaTheta);
    void adjustPhi(float deltaPhi);
    void adjustRadius(float deltaRadius) override;

    [[nodiscard]]
    glm::mat4 viewMatrix() override;

    [[nodiscard]]
    glm::vec3 position() override;

    std::string toString() const override;

private:

    void updateView();

    SceneTransform* mTarget;

    float mDistance{};
    float mMinDistance{};
    float mMaxDistance{};
    float mTheta = 0.0f;
    float mPhi = 0.0f;

    glm::mat4 mViewMatrix {};
    glm::vec3 mTargetPosition {};

    SceneTransform mFallbackTarget;
};
