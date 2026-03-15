#include "FreeCamera.hpp"
#include "utils/logger.h"

#include <algorithm>

FreeCamera::FreeCamera()
    : FreeCamera(Params{})
{
}

// Convert degrees to radians for internal storage
FreeCamera::FreeCamera(Params const& params)
    : BaseCamera(glm::radians(45.0f), 1.0f)
    , _front(0.0f, 0.0f, -1.0f)
    , _worldUp(params.initialWorldUp)
    , _yaw(glm::radians(params.initialYaw))
    , _pitch(glm::radians(params.initialPitch))
    , _movementSpeed(params.movementSpeed)
    , _mouseSensitivity(params.mouseSensitivity)
    , _constrainPitch(params.constrainPitch)
{
    _position = params.initialPosition;
    _up = params.initialWorldUp;
    updateCameraVectors();
}

void FreeCamera::processKeyboard(Movement direction, float deltaTime)
{
    float velocity = _movementSpeed * deltaTime;
    
    switch (direction)
    {
    case Movement::FORWARD:
        _position += _front * velocity;
        break;
    case Movement::BACKWARD:
        _position -= _front * velocity;
        break;
    case Movement::LEFT:
        _position -= _right * velocity;
        break;
    case Movement::RIGHT:
        _position += _right * velocity;
        break;
    case Movement::UP:
        _position += _up * velocity;
        break;
    case Movement::DOWN:
        _position -= _up * velocity;
        break;
    }
    
    _isDirty = true;
}

void FreeCamera::processMouseMovement(float xOffset, float yOffset)
{
    // Apply sensitivity and convert from degrees to radians
    xOffset *= _mouseSensitivity;
    yOffset *= _mouseSensitivity;
    
    _yaw += glm::radians(xOffset);
    _pitch += glm::radians(yOffset);

    // Constrain pitch to prevent screen flip (in radians)
    if (_constrainPitch)
    {
        _pitch = std::clamp(_pitch, glm::radians(-89.0f), glm::radians(89.0f));
    }

    // Update Front, Right and Up vectors using the updated Euler angles
    updateCameraVectors();
    _isDirty = true;
}

glm::mat4 FreeCamera::getViewMatrix()
{
    // Returns the view matrix calculated using Euler angles and LookAt matrix
    return glm::lookAt(_position, _position + _front, _up);
}

glm::vec3 FreeCamera::getPosition()
{
    return _position;
}


std::string FreeCamera::toString() const
{
    return fmt::format(
        "Free Camera\n"
        "Pos: ({:.1f}, {:.1f}, {:.1f})\n"
        "FOV: {:.1f}  Speed: {:.1f}\n"
        "Yaw: {:.1f}  Pitch: {:.1f}",
        _position.x, _position.y, _position.z,
        glm::degrees(_fov), _movementSpeed,
        glm::degrees(_yaw), glm::degrees(_pitch)
    );
}

void FreeCamera::updateCameraVectors()
{
    // Calculate the new Front vector from Euler angles (already in radians)
    glm::vec3 front;
    front.x = cos(_yaw) * cos(_pitch);
    front.y = sin(_pitch);
    front.z = sin(_yaw) * cos(_pitch);
    _front = glm::normalize(front);
    
    // Re-calculate the Right and Up vectors
    // Normalize the vectors because their length gets closer to 0 the more you look up or down
    _right = glm::normalize(glm::cross(_front, _worldUp));
    _up = glm::normalize(glm::cross(_right, _front));
}
