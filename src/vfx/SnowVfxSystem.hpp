#pragma once

#include "TerrainSnowSampler.hpp"
#include "ecs/System.hpp"

#include <memory>

class SnowVfxSystem : public System {
public:
    SnowVfxSystem();

    void update(float deltaTime);
    void setTerrainSampler(std::shared_ptr<TerrainSnowSampler> sampler);

private:
    std::shared_ptr<TerrainSnowSampler> terrainSampler;
};
