#include "FreeCamera.hpp"

#include <algorithm>
#include <fmt/format.h>

FreeCamera::FreeCamera()
    : FreeCamera(Params{})
{
}

// Convert degrees to radians for internal storage
FreeCamera::FreeCamera(Params const& params)
    : BaseCamera(glm::radians(45.0f))
    , mFront(0.0f, 0.0f, -1.0f)
    , mWorldUp(params.initialWorldUp)
    , mYaw(glm::radians(params.initialYaw))
    , mPitch(glm::radians(params.initialPitch))
    , mMovementSpeed(params.movementSpeed)
    , mMouseSensitivity(params.mouseSensitivity)
    , mConstrainPitch(params.constrainPitch)
{
    mPosition = params.initialPosition;
    mUp = params.initialWorldUp;
    updateCameraVectors();
}

void FreeCamera::processKeyboard(Movement direction, float deltaTime)
{
    float velocity = mMovementSpeed * deltaTime;
    
    switch (direction)
    {
    case Movement::FORWARD:
        mPosition += mFront * velocity;
        break;
    case Movement::BACKWARD:
        mPosition -= mFront * velocity;
        break;
    case Movement::LEFT:
        mPosition -= mRight * velocity;
        break;
    case Movement::RIGHT:
        mPosition += mRight * velocity;
        break;
    case Movement::UP:
        mPosition += mUp * velocity;
        break;
    case Movement::DOWN:
        mPosition -= mUp * velocity;
        break;
    }
    
    mIsDirty = true;
}

void FreeCamera::processMouseMovement(float xOffset, float yOffset)
{
    // Apply sensitivity and convert from degrees to radians
    xOffset *= mMouseSensitivity;
    yOffset *= mMouseSensitivity;
    
    mYaw += glm::radians(xOffset);
    mPitch += glm::radians(yOffset);

    // Constrain pitch to prevent screen flip (in radians)
    if (mConstrainPitch)
    {
        mPitch = std::clamp(mPitch, glm::radians(-89.0f), glm::radians(89.0f));
    }

    // Update Front, Right and Up vectors using the updated Euler angles
    updateCameraVectors();
    mIsDirty = true;
}

void FreeCamera::position(glm::vec3 newPosition)
{
    mPosition = newPosition;
    mIsDirty = true;
}

glm::mat4 FreeCamera::viewMatrix()
{
    // Returns the view matrix calculated using Euler angles and LookAt matrix
    return glm::lookAt(mPosition, mPosition + mFront, mUp);
}

std::string FreeCamera::toString() const
{
    return fmt::format(
        "Free Camera\n"
        "Pos: ({:.1f}, {:.1f}, {:.1f})\n"
        "FOV: {:.1f}  Speed: {:.1f}\n"
        "Yaw: {:.1f}  Pitch: {:.1f}",
        mPosition.x, mPosition.y, mPosition.z,
        glm::degrees(mFov), mMovementSpeed,
        glm::degrees(mYaw), glm::degrees(mPitch)
    );
}

void FreeCamera::updateCameraVectors()
{
    // Calculate the new Front vector from Euler angles (already in radians)
    glm::vec3 front;
    front.x = cos(mYaw) * cos(mPitch);
    front.y = sin(mPitch);
    front.z = sin(mYaw) * cos(mPitch);
    mFront = glm::normalize(front);
    
    // Re-calculate the Right and Up vectors
    // Normalize the vectors because their length gets closer to 0 the more you look up or down
    mRight = glm::normalize(glm::cross(mFront, mWorldUp));
    mUp = glm::normalize(glm::cross(mRight, mFront));
}
