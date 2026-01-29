#include "TurnTableCamera.hpp"

#include "utils/logger.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>

TurnTableCamera::TurnTableCamera()
    : TurnTableCamera(Params{})
{
}

TurnTableCamera::TurnTableCamera(Params const &params)
    : TurnTableCamera(*new Transform(), params)
{
    _target->setPosition({0,0,0});
}

TurnTableCamera::TurnTableCamera(Transform & target)
    : TurnTableCamera(target, Params{})
{
}

TurnTableCamera::TurnTableCamera(Transform & target, Params const &params)
{
    _target = &target;

    _distance = params.defaultDistance;
    _minDistance = params.minDistance;
    _maxDistance = params.maxDistance;
}

void TurnTableCamera::ChangeTarget(Transform &target)
{
    _target = &target;
    _isDirty = true;
}

glm::mat4 TurnTableCamera::ViewMatrix()
{
    UpdateViewMatrix();
    return _viewMatrix;
}

glm::vec3 TurnTableCamera::Position()
{
    UpdateViewMatrix();
    return _position;
}

void TurnTableCamera::UpdateViewMatrix()
{
    if (_isDirty == true || _targetPosition != _target->getWorldPosition())
    {
        _isDirty = false;

        auto const hRot = glm::rotate(glm::mat4(1.0f), _theta, math::transform::UpVec3);
        auto const vRot = glm::rotate(glm::mat4(1.0f), _phi, math::transform::RightVec3);

        _position = glm::vec3(hRot * vRot * glm::vec4{math::transform::ForwardVec3, 0.0f}) * _distance;

        // Center the camera around the target
        _targetPosition = _target->getWorldPosition();
        _position += _targetPosition;

        _viewMatrix = glm::lookAt(_position, _target->getWorldPosition(), math::transform::UpVec3);
    }
}

void TurnTableCamera::ChangeTheta(float const deltaTheta)
{
    auto newTheta = _theta + deltaTheta;
    if (newTheta != _theta)
    {
        _theta = newTheta;
        _isDirty = true;
    }
}

void TurnTableCamera::ChangePhi(float const deltaPhi)
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

void TurnTableCamera::ChangeRadius(float const deltaRadius)
{
    float const newDistance =  std::clamp(_distance + deltaRadius, _minDistance, _maxDistance);
    if (newDistance != _distance)
    {
        _isDirty = true;
        _distance = newDistance;
    }
}

// The FOV when in perspective view
void TurnTableCamera::ChangeFOV(float deltaFOV)
{
    float const newFOV =  std::clamp(_fov + deltaFOV, 1.0f, 180.0f);
    if (newFOV != _fov)
    {
        _isDirty = true;
        _fov = newFOV;
    }
}

// The scale when in orthographic view
void TurnTableCamera::ChangeScale(float deltaScale)
{
    float const newScale =  std::clamp(_scale + deltaScale, 0.01f, 1000.0f);
    if (newScale != _scale)
    {
        _isDirty = true;
        _scale = newScale;
    }
}

// Package camera stats to print in ImGui
CameraStats TurnTableCamera::getStats()
{
    return CameraStats(_position, _targetPosition, _distance, _fov, _scale);
}
