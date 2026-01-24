#pragma once

#define GLM_ENABLE_EXPERIMENTAL // Required for quaternion
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class Transform {
public:
    glm::vec3 pos;
    glm::quat rot;
};
