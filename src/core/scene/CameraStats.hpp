#pragma once

#include "utils/logger.h"

struct CameraStats {
    CameraStats() {}
    CameraStats(glm::vec3 cam_pos, glm::vec3 target_pos, float radius, float fov, float scale)
        :camPos(cam_pos), target(target_pos), radius(radius), fov(fov), scale(scale) {}
    
    glm::vec3 camPos, target;
    float radius, fov, scale, aspect;
};
