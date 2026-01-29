#include "Camera.hpp"

Camera::Camera() : Camera(glm::vec3(0.0f, 1.0f, 0.0f)) {}

Camera::Camera(glm::vec3 up_vec)
    : BaseCamera(120.0f, 1.0f)
{
    _up = up_vec;
    _position = glm::vec3(0.0f, 0.0f, 1.0f);
    camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    _radius = 3.0f;
    _phi = 0.0f;
    _theta = glm::radians(180.0f);
}

// The actual distance of the camera from the origin
void Camera::adjustRadius(float amount) {
    _radius = bound(_radius + amount, 0.1f, 1000.0f);
    _isDirty = true;
}

// Positive speed moves up, negative moves down
void Camera::moveVertically(float speed)
{
    // logger::debug("Move vertical");
    if (isCamera2D) {
        _position += glm::vec3(0.0f, speed / 2, 0.0f);
    } else {
        _phi = bound(_phi + speed, glm::radians(-89.9f), glm::radians(89.9f));
    }
}

// Positive speed moves right, negative moves left
void Camera::moveHorizontally(float speed)
{
    // logger::debug("Move horizonal");
    if (isCamera2D) {
        _position += glm::vec3(speed / 2, 0.0f, 0.0f);
    } else {
        _theta += speed;
    }
}

// Returns the view matrix for a turn table camera
glm::mat4 Camera::getViewMatrix()
{
    if (isCamera2D) {
        _target = _position + camFront;
    } else {
        prepareTurnTable();
    }

    return glm::lookAt(_position, _target, _up);
}

// Package camera stats to print in ImGui
CameraStats Camera::getStats() {
    return CameraStats(_position, _target, _radius, _fov, _scale);
}

// Set the position and target for a turn table camera
void Camera::prepareTurnTable()
{
    // Spherical Coordinates with y and z swapped
    // x = r*cos(theta)*sin(phi)
    // y = r*cos(phi)
    // z = r*sin(theta)*sin(phi)
    float phi = _phi - glm::radians(90.0f);
    float camX = _radius * sin(_theta) * sin(phi);
    float camY = _radius * cos(phi);
    float camZ = _radius * cos(_theta) * sin(phi);
    _position = glm::vec3(camX, camY, camZ);
    _target = glm::vec3(0.0f);
}

// Helper function to bound a value between a min and max
// Made in the first assignment and copied here
float Camera::bound(float value, float min, float max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}
