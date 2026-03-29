#pragma once

#include <glm/glm.hpp>

// Presets used by gameplay code to select default emitter behavior.
enum class SnowEmitterPreset {
    AvalancheFront,
    WheelTrail,
    WheelSplash,
    Custom
};

// ECS data component for all snow-related emitters.
struct SnowEmitter {
    bool enabled = true;
    SnowEmitterPreset preset = SnowEmitterPreset::Custom;

    float spawnRate = 0.0f;
    float particleLifetimeSec = 0.0f;
    float particleSize = 0.0f;

    glm::vec3 localOffset{0.0f, 0.0f, 0.0f};
};
