#pragma once

#include "utils/logger.h"

struct CameraStats {
    CameraStats()
        : CameraStats(glm::vec3(0.0f, 0.0f, 1.0f),
                      glm::vec3(0.0f, 0.0f, 0.0f)) {}

    CameraStats(glm::vec3 position, glm::vec3 target)
        : position(position)
        , target(target)
        , distance(1.f)
        , fov(90.f)
        , scale(1.f)
        , aspect(1.0f)
        , yaw(0.0f)
        , pitch(0.0f) {}
    
    // Position fields (used by different camera types)
    glm::vec3 position;  // Camera position
    glm::vec3 target;    // Look-at target
    
    // Camera properties
    float distance; // Distance from target
    float fov;      // Field of view
    float scale;    // Scale factor
    float aspect;   // Aspect ratio
    
    // Euler angles
    float yaw;    // Horizontal rotation
    float pitch;  // Vertical rotation
};
