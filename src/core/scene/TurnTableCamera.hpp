#pragma once

#include "utils/math.hpp"

#include "BaseCamera.hpp"
#include "Transform.hpp"
#include "CameraStats.hpp"

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
    explicit TurnTableCamera(Params const &params);

    // Angles are in radians
    explicit TurnTableCamera(SceneTransform & target);
    explicit TurnTableCamera(SceneTransform & target, const Params &params);

    void setTarget(SceneTransform & target);

    void adjustTheta(float deltaTheta);
    void adjustPhi(float deltaPhi);
    void adjustRadius(float deltaRadius) override;

    [[nodiscard]]
    glm::mat4 getViewMatrix() override;

    [[nodiscard]]
    glm::vec3 getPosition() override;

    CameraStats getStats() override;

private:

    void updateViewMatrix();

    SceneTransform * _target;

    float _distance{};
    float _minDistance{};
    float _maxDistance{};

    glm::mat4 _viewMatrix {};
    glm::vec3 _targetPosition {};

    SceneTransform fallbackTarget;
};
