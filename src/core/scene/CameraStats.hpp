#pragma once

#include "utils/logger.h"

struct CameraStats {
CameraStats()
    : CameraStats(glm::vec3(0.0f, 0.0f, 1.0f),
                  glm::vec3(0.0f, 0.0f, 0.0f),
                  5.0f, 45.0f, 1.0f) {}

CameraStats(glm::vec3 cam_pos, glm::vec3 target_pos, float radius, float fov, float scale)
    : camPos(cam_pos), target(target_pos), radius(radius), fov(fov), scale(scale), aspect(1.0f) {}
    
    glm::vec3 camPos, target;
    float radius, fov, scale, aspect;
};
