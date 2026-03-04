#pragma once

#define GLM_ENABLE_EXPERIMENTAL // Required for quaternion
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct PhysxTransform {
    glm::vec3 pos;
    glm::quat rot;
    glm::vec3 scale{ 1.f };

    glm::mat4 toMatrix() const {
        return glm::translate(glm::mat4(1.f), pos) *
            glm::toMat4(rot) *
            glm::scale(glm::mat4(1.f), scale);
    }

    glm::vec3 forward() const { return rot * glm::vec3(0, 0, 1); }
    glm::vec3 right() const { return rot * glm::vec3(1, 0, 0); }
    glm::vec3 up() const { return rot * glm::vec3(0, 1, 0); }
};
