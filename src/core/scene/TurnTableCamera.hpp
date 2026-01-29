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
    explicit TurnTableCamera(Transform & target);
    explicit TurnTableCamera(Transform & target, const Params &params);

    // For the bonus camera needs to be able to follow a target
    void setTarget(Transform & target);

    void adjustTheta(float deltaTheta);
    void adjustPhi(float deltaPhi);
    void adjustRadius(float deltaRadius);

    [[nodiscard]]
    glm::mat4 getViewMatrix() override;

    [[nodiscard]]
    glm::vec3 getPosition() override;

    CameraStats getStats() override;

    glm::mat4 viewMatrix();
    glm::vec3 position();

private:

    void updateViewMatrix();

    Transform * _target;

    float _distance{};
    float _minDistance{};
    float _maxDistance{};

    float _theta {};
    float _phi {};

    glm::mat4 _viewMatrix {};
    glm::vec3 _position {};
    glm::vec3 _targetPosition {};
};
