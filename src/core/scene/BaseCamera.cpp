#include "BaseCamera.hpp"
#include <algorithm>

BaseCamera::BaseCamera()
    : BaseCamera(glm::radians(60.0f))  // Convert to radians
{
}

BaseCamera::BaseCamera(float fov)
    : _fov(fov)
    , _up(0.0f, 1.0f, 0.0f)
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

void BaseCamera::adjustRadius(float deltaRadius)
{
    (void)deltaRadius;
}

glm::vec3 BaseCamera::forward()
{
    glm::mat4 const view = getViewMatrix();
    return glm::normalize(-glm::vec3(view[0][2], view[1][2], view[2][2]));
}

glm::vec3 BaseCamera::right()
{
    glm::mat4 const view = getViewMatrix();
    return glm::normalize(glm::vec3(view[0][0], view[1][0], view[2][0]));
}

void BaseCamera::setFOV(float fov)
{
    // fov parameter is in degrees for user convenience, convert to radians for storage
    _fov = std::clamp(glm::radians(fov), glm::radians(1.0f), glm::radians(180.0f));
    _isDirty = true;
}

std::string BaseCamera::toString() const
{
    return "Base Camera";
}
