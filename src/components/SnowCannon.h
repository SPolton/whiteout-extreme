#pragma once
#include <memory>
#include <glm/glm.hpp>

struct SnowCannon {
    glm::quat restRotation;     // Original roation
    Entity currentTarget;       // Who aggroed the snow cannon
    float aggroTimer = 0.0f;    // Aggro time left
    float fireCooldown = 0.5f;
    float coolingPower = 10.0f; // Impact on the engines
    float oscillationTimer = 0.0f;
};
