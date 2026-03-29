#pragma once

#include <glm/glm.hpp>

class TerrainSnowSampler {
public:
    TerrainSnowSampler() = default;
    ~TerrainSnowSampler() = default;

    // Placeholder for Step 1. Real terrain queries arrive in later steps.
    float sampleHeight(float worldX, float worldZ) const;
    glm::vec3 sampleNormal(float worldX, float worldZ) const;
};
