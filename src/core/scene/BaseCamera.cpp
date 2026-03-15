#include "BaseCamera.hpp"
#include <algorithm>

BaseCamera::BaseCamera()
    : BaseCamera(glm::radians(60.0f))  // Convert to radians
{
}

BaseCamera::BaseCamera(float fov)
    : mFov(fov)
    , mUp(0.0f, 1.0f, 0.0f)
    , mPosition(0.0f, 0.0f, 1.0f)
    , mIsDirty(true)
{
}

void BaseCamera::adjustFOV(float deltaFOV)
{
    // deltaFOV is in degrees for user convenience, convert to radians
    float const newFOV = std::clamp(mFov + glm::radians(deltaFOV), glm::radians(1.0f), glm::radians(180.0f));
    if (newFOV != mFov)
    {
        mFov = newFOV;
        mIsDirty = true;
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
    mFov = std::clamp(glm::radians(fov), glm::radians(1.0f), glm::radians(180.0f));
    mIsDirty = true;
}

std::string BaseCamera::toString() const
{
    return "Base Camera";
}
