#include "SnowVfxSystem.hpp"

#include "ecs/Coordinator.hpp"
#include "utils/math.hpp"

#include <algorithm>
#include <unordered_set>

// https://learnopengl.com/In-Practice/2D-Game/Particles

extern Coordinator gCoordinator;

SnowVfxSystem::SnowVfxSystem()
    : particles(kMaxParticles)
{
}

void SnowVfxSystem::update(float deltaTime)
{
    if (deltaTime <= 0.0f) {
        return;
    }

    cleanupEmitterAccumulators();
    spawnParticles(deltaTime);
    updateParticles(deltaTime);
    rebuildSnowFrame();
}

void SnowVfxSystem::setTerrainSampler(std::shared_ptr<TerrainSnowSampler> sampler)
{
    terrainSampler = std::move(sampler);
}

std::size_t SnowVfxSystem::aliveParticleCount() const
{
    return static_cast<std::size_t>(std::count_if(
        particles.begin(), particles.end(), [](const SnowParticle& p) { return p.lifeSec > 0.0f; }));
}

const SnowFrame& SnowVfxSystem::snowFrame() const
{
    return currentSnowFrame;
}

std::size_t SnowVfxSystem::firstUnusedParticle()
{
    for (std::size_t i = lastUsedParticle; i < particles.size(); ++i) {
        if (particles[i].lifeSec <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }

    for (std::size_t i = 0; i < lastUsedParticle; ++i) {
        if (particles[i].lifeSec <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }

    lastUsedParticle = 0;
    return 0;
}

void SnowVfxSystem::cleanupEmitterAccumulators()
{
    std::unordered_set<Entity> activeEmitters;
    activeEmitters.reserve(mEntities.size());

    for (const Entity entity : mEntities) {
        activeEmitters.insert(entity);
    }

    for (auto it = emitterSpawnAccumulator.begin(); it != emitterSpawnAccumulator.end();) {
        if (!activeEmitters.contains(it->first)) {
            it = emitterSpawnAccumulator.erase(it);
            continue;
        }

        ++it;
    }
}

void SnowVfxSystem::spawnParticles(float deltaTime)
{
    for (auto const& entity : mEntities) {
        bool hasGridBox = gCoordinator.HasComponent<SnowEmitterGridBox>(entity);
        if (hasGridBox) {
            spawnParticlesFromGridBox(entity, deltaTime);
        } else {
            spawnParticlesFromEmitter(entity, deltaTime);
        }
    }
}

void SnowVfxSystem::spawnParticlesFromGridBox(Entity entity, float deltaTime)
{
    spawnParticlesFromEmitter(entity, deltaTime);
}

void SnowVfxSystem::spawnParticlesFromEmitter(Entity entity, float deltaTime)
{
    auto& emitter = gCoordinator.GetComponent<SnowEmitter>(entity);
    auto& transform = gCoordinator.GetComponent<PhysxTransform>(entity);

    if (!emitter.isValid()) return;

    float& accumulator = emitterSpawnAccumulator[entity];
    accumulator += emitter.spawnRate * deltaTime;

    int spawnCount = static_cast<int>(accumulator);
    if (spawnCount <= 0) return;

    if (spawnCount > kMaxSpawnPerEmitterPerStep) {
        spawnCount = kMaxSpawnPerEmitterPerStep;
        accumulator = 0.0f;
    }

    accumulator -= static_cast<float>(spawnCount);

    for (int i = 0; i < spawnCount; ++i) {
        spawnParticleAt(emitter, transform);
    }
}

void SnowVfxSystem::spawnParticleAt(SnowEmitter const& emitter, PhysxTransform const& transform)
{
    std::size_t idx = firstUnusedParticle();
    SnowParticle& particle = particles[idx];

    const float jitterX = math::random::RandomFloat(-jitterAmount, jitterAmount);
    const float jitterZ = math::random::RandomFloat(-jitterAmount, jitterAmount);
    const float upVelocity = math::random::RandomFloat(upVelocityMin, upVelocityMax);
    const float driftVelocityX = math::random::RandomFloat(driftVelocityMin, driftVelocityMax);
    const float driftVelocityZ = math::random::RandomFloat(driftVelocityMin, driftVelocityMax);

    particle.position = transform.pos + (transform.rot * emitter.localOffset) + glm::vec3(jitterX, 0.0f, jitterZ);
    particle.velocity = transform.rot * glm::vec3(driftVelocityX, upVelocity, driftVelocityZ);
    particle.lifeSec = emitter.particleLifetimeSec;
    particle.size = std::max(emitter.particleSize, 0.05f);
}

void SnowVfxSystem::updateParticles(float deltaTime)
{
    for (SnowParticle& particle : particles) {
        if (!particle.isValid()) continue;

        particle.lifeSec -= deltaTime;
        if (!particle.isValid()) continue;

        particle.velocity += glm::vec3(0.0f, -9.81f, 0.0f) * deltaTime * 0.25f;
        particle.position += particle.velocity * deltaTime;
    }
}

void SnowVfxSystem::rebuildSnowFrame()
{
    currentSnowFrame.particles.clear();
    currentSnowFrame.particles.reserve(aliveParticleCount());

    for (const SnowParticle& particle : particles) {
        if (!particle.isValid()) continue;

        currentSnowFrame.particles.push_back(particle);
    }
}

std::vector<glm::vec3> SnowVfxSystem::generateGridSpawnPoints(SnowEmitterGridBox const& gridBox)
{
    std::vector<glm::vec3> points;

    if (!gridBox.isValid()) {
        return points;
    }

    // Compute cell dimensions.
    const glm::vec3 cellSize = gridBox.localBoxSize / glm::vec3(gridBox.gridResolution);
    const glm::vec3 halfBox = gridBox.localBoxSize * 0.5f;

    // Generate grid cell centers in local space, origin at box center.
    for (int x = 0; x < gridBox.gridResolution.x; ++x) {
        for (int y = 0; y < gridBox.gridResolution.y; ++y) {
            for (int z = 0; z < gridBox.gridResolution.z; ++z) {
                // Check pattern filter.
                if (gridBox.pattern == SnowEmitterGridPattern::Checkerboard) {
                    if ((x + y + z) % 2 != 0) {
                        continue;
                    }
                }

                // Compute cell center.
                const glm::vec3 cellCenter = -halfBox + cellSize * glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f);
                points.push_back(gridBox.localOffset + cellCenter);
            }
        }
    }

    return points;
}
