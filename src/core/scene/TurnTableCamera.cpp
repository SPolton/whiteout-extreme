#include "TurnTableCamera.hpp"

#include "utils/logger.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>

TurnTableCamera::TurnTableCamera()
    : TurnTableCamera(Params{})
{
}

TurnTableCamera::TurnTableCamera(Params const &params)
    : TurnTableCamera(fallbackTarget, params)
{
}

TurnTableCamera::TurnTableCamera(SceneTransform & target)
    : TurnTableCamera(target, Params{})
{
}

TurnTableCamera::TurnTableCamera(SceneTransform & target, Params const &params)
    : BaseCamera()
{
    _target = &target;

    _distance = params.defaultDistance;
    _minDistance = params.minDistance;
    _maxDistance = params.maxDistance;
}

void TurnTableCamera::setTarget(SceneTransform &target)
{
    _target = &target;
    _isDirty = true;
}

glm::mat4 TurnTableCamera::getViewMatrix()
{
    updateViewMatrix();
    return _viewMatrix;
}

glm::vec3 TurnTableCamera::getPosition()
{
    updateViewMatrix();
    return _position;
}

void TurnTableCamera::updateViewMatrix()
{
    if (_isDirty == true || _targetPosition != _target->getWorldPosition())
    {
        _isDirty = false;

        auto const hRot = glm::rotate(glm::mat4(1.0f), _theta, _up);
        auto const vRot = glm::rotate(glm::mat4(1.0f), _phi, math::transform::RightVec3);

        _position = glm::vec3(hRot * vRot * glm::vec4{math::transform::ForwardVec3, 0.0f}) * _distance;

        // Center the camera around the target
        _targetPosition = _target->getWorldPosition();
        _position += _targetPosition;

        _viewMatrix = glm::lookAt(_position, _target->getWorldPosition(), math::transform::UpVec3);
    }
}

void TurnTableCamera::adjustTheta(float const deltaTheta)
{
    auto newTheta = _theta + deltaTheta;
    // auto newTheta = std::fmod(_theta + deltaTheta, 2*M_PI);
    if (newTheta != _theta)
    {
        _theta = newTheta;
        _isDirty = true;
    }
}

void TurnTableCamera::adjustPhi(float const deltaPhi)
{
    float const newPhi = std::clamp(
        _phi + deltaPhi,
        -glm::pi<float>() * 0.49f,
        glm::pi<float>() * 0.49f
    );
    if (newPhi != _phi)
    {
        _isDirty = true;
        _phi = newPhi;
    }
}

void TurnTableCamera::adjustRadius(float const deltaRadius)
{
    float const newDistance =  std::clamp(_distance + deltaRadius, _minDistance, _maxDistance);
    if (newDistance != _distance)
    {
        _isDirty = true;
        _distance = newDistance;
    }
}

CameraStats TurnTableCamera::getStats()
{
    CameraStats stats = CameraStats(_position, _targetPosition);
    stats.distance = _distance;
    stats.fov = glm::degrees(_fov);
    stats.scale = _scale;
    stats.yaw = glm::degrees(_theta);
    stats.pitch = glm::degrees(_phi);
    return stats;
}
