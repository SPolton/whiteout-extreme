#pragma once

#include "utils/math.hpp"

#include "Transform.hpp"
#include "CameraStats.hpp"

class TurnTableCamera {
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
    void ChangeTarget(Transform & target);

    void ChangeTheta(float deltaTheta);
    void ChangePhi(float deltaPhi);
    void ChangeRadius(float deltaRadius);

    void ChangeFOV(float deltaFOV);
    void ChangeScale(float deltaScale);
    
    float getFOV() { return _fov; }
    float getScale() { return _scale; }

    CameraStats getStats();

    [[nodiscard]]
    glm::mat4 ViewMatrix();

    [[nodiscard]]
    glm::vec3 Position();

private:

    void UpdateViewMatrix();

    Transform * _target;

    float _distance{};
    float _minDistance{};
    float _maxDistance{};

    float _theta {};
    float _phi {};

    float _fov = 120.0f;
    float _scale = 1.0f;

    bool _isDirty = true;

    glm::mat4 _viewMatrix {};
    glm::vec3 _position {};
    glm::vec3 _targetPosition {};
};
