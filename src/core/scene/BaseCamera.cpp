#include "BaseCamera.hpp"
#include <algorithm>

BaseCamera::BaseCamera()
    : BaseCamera(120.0f, 1.0f)
{
}

BaseCamera::BaseCamera(float fov, float scale)
    : _fov(fov)
    , _scale(scale)
    , _isDirty(true)
{
}

void BaseCamera::adjustFOV(float deltaFOV)
{
    float const newFOV = clampValue(_fov + deltaFOV, 1.0f, 180.0f);
    if (newFOV != _fov)
    {
        _fov = newFOV;
        _isDirty = true;
    }
}

void BaseCamera::adjustScale(float deltaScale)
{
    float const newScale = clampValue(_scale + deltaScale, 0.01f, 1000.0f);
    if (newScale != _scale)
    {
        _scale = newScale;
        _isDirty = true;
    }
}

void BaseCamera::setFOV(float fov)
{
    _fov = clampValue(fov, 1.0f, 180.0f);
    _isDirty = true;
}

void BaseCamera::setScale(float scale)
{
    _scale = clampValue(scale, 0.01f, 1000.0f);
    _isDirty = true;
}

float BaseCamera::clampValue(float value, float min, float max)
{
    return std::clamp(value, min, max);
}
