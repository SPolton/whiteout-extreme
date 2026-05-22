#pragma once

#include "ecs/Types.hpp"
#include <glm/glm.hpp>

struct SnowBall {
    Entity launcher{};  // Who shot the snowball
    float damage = 5.0f;
    float maxDistance = 100.0f;
    glm::vec3 spawnPos{};
};
