#pragma once

#include "SnowParticle.hpp"
#include "TerrainSnowSampler.hpp"
#include "ecs/System.hpp"
#include "components/SnowEmitter.h"

#include <glm/glm.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

class SnowVfxSystem : public System {
public:
    SnowVfxSystem();

    void update(float deltaTime);
    void setTerrainSampler(std::shared_ptr<TerrainSnowSampler> sampler);
    std::size_t aliveParticleCount() const;
    const SnowFrame& snowFrame() const;

private:
    static constexpr std::size_t kMaxParticles = 4096;
    static constexpr int kMaxSpawnPerEmitterPerStep = 128;

    std::vector<SnowParticle> particles;
    std::unordered_map<Entity, float> emitterSpawnAccumulator;
    std::size_t lastUsedParticle = 0;

    std::shared_ptr<TerrainSnowSampler> terrainSampler;
    SnowFrame currentSnowFrame;

    std::size_t firstUnusedParticle();
    void cleanupEmitterAccumulators();
    void spawnParticles(float deltaTime);
    void updateParticles(float deltaTime);
    void rebuildSnowFrame();

    // Generate deterministic grid spawn points from box parameters.
    static std::vector<glm::vec3> generateGridSpawnPoints(SnowEmitterGridBox const& gridBox);

    float jitterAmount = 3.5f;
    float upVelocityMin = 1.0f;
    float upVelocityMax = 5.0f;
    float driftVelocityMin = -0.9f;
    float driftVelocityMax = 0.9f;
};
