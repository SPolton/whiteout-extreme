#include "SnowVfxSystem.hpp"

SnowVfxSystem::SnowVfxSystem() = default;

void SnowVfxSystem::update(float deltaTime)
{
    (void)deltaTime;
}

void SnowVfxSystem::setTerrainSampler(std::shared_ptr<TerrainSnowSampler> sampler)
{
    terrainSampler = std::move(sampler);
}
