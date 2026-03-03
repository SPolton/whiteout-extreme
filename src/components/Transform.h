#pragma once

#define GLM_ENABLE_EXPERIMENTAL // Required for quaternion
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class PhysxTransform {
public:
    glm::vec3 pos;
    glm::quat rot;
    glm::vec3 scale{ 1.f };

    glm::vec3 getForwardVector() const {
        return rot * glm::vec3(0.0f, 0.0f, 1.0f);
    }

    glm::vec3 getRightVector() const {
        return rot * glm::vec3(1.0f, 0.0f, 0.0f);
    }
};
