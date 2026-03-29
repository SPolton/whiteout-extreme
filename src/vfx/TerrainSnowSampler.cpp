#include "TerrainSnowSampler.hpp"

float TerrainSnowSampler::sampleHeight(float worldX, float worldZ) const
{
    (void)worldX;
    (void)worldZ;
    return 0.0f;
}

glm::vec3 TerrainSnowSampler::sampleNormal(float worldX, float worldZ) const
{
    (void)worldX;
    (void)worldZ;
    return glm::vec3(0.0f, 1.0f, 0.0f);
}
