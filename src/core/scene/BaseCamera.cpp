#include "BaseCamera.hpp"
#include <algorithm>

BaseCamera::BaseCamera()
    : BaseCamera(glm::radians(60.0f), 1.0f)  // Convert to radians
{
}

BaseCamera::BaseCamera(float fov, float scale)
    : _fov(fov)
    , _scale(scale)
    , _radius(3.0f)
    , _theta(0.0f)
    , _phi(0.0f)
    , _up(0.0f, 1.0f, 0.0f)
    , _target(0.0f)
    , _position(0.0f, 0.0f, 1.0f)
    , _isDirty(true)
{
}

void BaseCamera::adjustFOV(float deltaFOV)
{
    // deltaFOV is in degrees for user convenience, convert to radians
    float const newFOV = std::clamp(_fov + glm::radians(deltaFOV), glm::radians(1.0f), glm::radians(180.0f));
    if (newFOV != _fov)
    {
        _fov = newFOV;
        _isDirty = true;
    }
}

void BaseCamera::adjustScale(float deltaScale)
{
    float const newScale = std::clamp(_scale + deltaScale, 0.01f, 1000.0f);
    if (newScale != _scale)
    {
        _scale = newScale;
        _isDirty = true;
    }
}

void BaseCamera::adjustRadius(float deltaRadius)
{
    float const newRadius = std::clamp(_radius + deltaRadius, 0.1f, 1000.0f);
    if (newRadius != _radius)
    {
        _radius = newRadius;
        _isDirty = true;
    }
}

void BaseCamera::setFOV(float fov)
{
    // fov parameter is in degrees for user convenience, convert to radians for storage
    _fov = std::clamp(glm::radians(fov), glm::radians(1.0f), glm::radians(180.0f));
    _isDirty = true;
}

void BaseCamera::setScale(float scale)
{
    _scale = std::clamp(scale, 0.01f, 1000.0f);
    _isDirty = true;
}

void BaseCamera::setRadius(float distance)
{
    _radius = std::clamp(distance, 0.1f, 1000.0f);
    _isDirty = true;
}

std::string BaseCamera::toString() const
{
    return "Base Camera";
}
