#pragma once

#include "TerrainSnowSampler.hpp"
#include "ecs/System.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

struct SnowParticle {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 velocity{0.0f, 0.0f, 0.0f};
    float lifeSec = 0.0f;
    float size = 0.0f;
};

class SnowVfxSystem : public System {
public:
    SnowVfxSystem();

    void update(float deltaTime);
    void setTerrainSampler(std::shared_ptr<TerrainSnowSampler> sampler);
    std::size_t aliveParticleCount() const;

private:
    static constexpr std::size_t kMaxParticles = 4096;

    std::vector<SnowParticle> particles;
    std::unordered_map<Entity, float> emitterSpawnAccumulator;
    std::size_t lastUsedParticle = 0;

    std::shared_ptr<TerrainSnowSampler> terrainSampler;

    std::size_t firstUnusedParticle();
    void spawnParticles(float deltaTime);
    void updateParticles(float deltaTime);
};
