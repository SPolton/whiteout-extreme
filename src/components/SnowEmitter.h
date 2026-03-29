#pragma once

#include <glm/glm.hpp>

// Presets used by gameplay code to select default emitter behavior.
enum class SnowEmitterPreset {
    AvalancheFront,
    WheelTrail,
    WheelSplash,
    Custom
};

// Grid pattern selection for boxed emitter layouts.
enum class SnowEmitterGridPattern {
    All,
    Checkerboard
};

// Optional particle overrides for boxed-grid emitters.
struct SnowEmitterParticleOverrides {
    bool enabled = false;

    float spawnRate = 0.0f;
    float particleLifetimeSec = 0.0f;
    float particleSize = 0.0f;
};

// Single ECS component describing a box and grid of logical emitter points.
struct SnowEmitterGridBox {
    bool enabled = true;

    // Dimensions of the local-space box volume (meters).
    glm::vec3 localBoxSize{1.0f, 1.0f, 1.0f};

    // Number of grid cells across each local axis.
    glm::ivec3 gridResolution{1, 1, 1};

    // Local-space offset from owning entity origin.
    glm::vec3 localOffset{0.0f, 0.0f, 0.0f};

    SnowEmitterGridPattern pattern = SnowEmitterGridPattern::Checkerboard;
    SnowEmitterParticleOverrides particleOverrides{};

    bool isValid() const
    {
        return enabled &&
            gridResolution.x >= 1 && gridResolution.y >= 1 && gridResolution.z >= 1 &&
            localBoxSize.x > 0.0f && localBoxSize.y > 0.0f && localBoxSize.z > 0.0f;
    }
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
