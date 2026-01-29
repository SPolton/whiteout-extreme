#include "BaseCamera.hpp"
#include <algorithm>

BaseCamera::BaseCamera()
    : BaseCamera(120.0f, 1.0f)
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
    float const newFOV = std::clamp(_fov + deltaFOV, 1.0f, 180.0f);
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
    _fov = std::clamp(fov, 1.0f, 180.0f);
    _isDirty = true;
}

void BaseCamera::setScale(float scale)
{
    _scale = std::clamp(scale, 0.01f, 1000.0f);
    _isDirty = true;
}

void BaseCamera::setRadius(float radius)
{
    _radius = std::clamp(radius, 0.1f, 1000.0f);
    _isDirty = true;
}
