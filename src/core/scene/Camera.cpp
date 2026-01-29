#include "Camera.hpp"

Camera::Camera() : Camera(glm::vec3(0.0f, 1.0f, 0.0f)) {}

// https://learnopengl.com/Getting-Started/Camera
Camera::Camera(glm::vec3 up_vec)
{
    camUp = up_vec;
    camPos = glm::vec3(0.0f, 0.0f, 1.0f);
    camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    radius = 3.0f;
    fov = 120.0f;
    scale = 1.0f;
    verticalAngle = 0.0f;
    horizontalAngle = glm::radians(180.0f);
}

// The fov when in perspective view
void Camera::adjustFOV(float amount) {
    fov = bound(fov + amount, 1.0f, 180.0f);
}

// The scale when in orthographic view
void Camera::adjustScale(float amount) {
    scale = bound(scale + amount, 0.01f, 100.0f);
}

// The actual distance of the camera from the origin
void Camera::adjustRadius(float amount) {
    radius = bound(radius + amount, 0.1f, 1000.0f);
}

// Positive speed moves up, negative moves down
void Camera::moveVertically(float speed)
{
    // logger::debug("Move vertical");
    if (isCamera2D) {
        camPos += glm::vec3(0.0f, speed / 2, 0.0f);
    } else {
        verticalAngle = bound(verticalAngle + speed, glm::radians(-89.9f), glm::radians(89.9f));
    }
}

// Positive speed moves right, negative moves left
void Camera::moveHorizontally(float speed)
{
    // logger::debug("Move horizonal");
    if (isCamera2D) {
        camPos += glm::vec3(speed / 2, 0.0f, 0.0f);
    } else {
        horizontalAngle += speed;
    }
}

// Returns the view matrix for a turn table camera
glm::mat4 Camera::getViewMatrix()
{
    if (isCamera2D) {
        target = camPos + camFront;
    } else {
        prepareTurnTable();
    }

    return glm::lookAt(camPos, target, camUp);
}

// Package camera stats to print in ImGui
CameraStats Camera::getStats() {
    return CameraStats(camPos, target, radius, fov, scale);
}

// Set the position and target for a turn table camera
void Camera::prepareTurnTable()
{
    // Spherical Coordinates with y and z swapped
    // x = r*cos(theta)*sin(phi)
    // y = r*cos(phi)
    // z = r*sin(theta)*sin(phi)
    float phi = verticalAngle - glm::radians(90.0f);
    float camX = radius * sin(horizontalAngle) * sin(phi);
    float camY = radius * cos(phi);
    float camZ = radius * cos(horizontalAngle) * sin(phi);
    camPos = glm::vec3(camX, camY, camZ);
    target = glm::vec3(0.0f);
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
