#include "SnowVfxSystem.hpp"

#include "components/SnowEmitter.h"
#include "components/Transform.h"
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
        auto& emitter = gCoordinator.GetComponent<SnowEmitter>(entity);
        auto& transform = gCoordinator.GetComponent<PhysxTransform>(entity);

        if (!emitter.enabled || emitter.spawnRate <= 0.0f || emitter.particleLifetimeSec <= 0.0f) {
            continue;
        }

        float& accumulator = emitterSpawnAccumulator[entity];
        accumulator += emitter.spawnRate * deltaTime;

        int spawnCount = static_cast<int>(accumulator);
        if (spawnCount <= 0) {
            continue;
        }

        if (spawnCount > kMaxSpawnPerEmitterPerStep) {
            spawnCount = kMaxSpawnPerEmitterPerStep;
            accumulator = 0.0f;
        }

        accumulator -= static_cast<float>(spawnCount);

        // particles movement if emmiter is nitro
        if (emitter.preset == SnowEmitterPreset::Nitro) {

            // we spawn two times the particles, one for left and one for right exhuast
            int nitroSpawnCount = spawnCount * 2;

            for (int i = 0; i < nitroSpawnCount; ++i) {
                std::size_t idx = firstUnusedParticle();
                SnowParticle& particle = particles[idx];

                // forward position of vehicle
                glm::vec3 forward = transform.rot * glm::vec3(0, 0, 1);
                // make particles really fast
                const float speed = 15.0f;
                // nitro particles moves in opposite direction (of forward) with specified speed
                particle.velocity = (-forward * speed);

                // if not at half point, spawn on one side
                if (i < spawnCount) {
                    // no jitter needed for nitro
                    particle.position = transform.pos + (transform.rot * emitter.localOffset);
                }
                // when half point has been hit, spawn on other side
                else {
                    particle.position = transform.pos + (transform.rot * emitter.localOffset2);
                }

                // set the other particle attributes
                particle.lifeSec = emitter.particleLifetimeSec;
                particle.size = std::max(emitter.particleSize, 0.05f);
                particle.color = glm::vec3(0.8f, 0.2f, 0.0f); // orange
            }
        }
        // otherwise its for avalanche
        else {
            for (int i = 0; i < spawnCount; ++i) {
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
                particle.color = glm::vec3(0.95f, 0.97f, 1.0f); // white
            }
        }
    }
}

void SnowVfxSystem::updateParticles(float deltaTime)
{
    for (SnowParticle& particle : particles) {
        if (particle.lifeSec <= 0.0f) {
            continue;
        }

        particle.lifeSec -= deltaTime;
        if (particle.lifeSec <= 0.0f) {
            particle.lifeSec = 0.0f;
            continue;
        }

        particle.velocity += glm::vec3(0.0f, -9.81f, 0.0f) * deltaTime * 0.25f;
        particle.position += particle.velocity * deltaTime;
    }
}

void SnowVfxSystem::rebuildSnowFrame()
{
    currentSnowFrame.particles.clear();
    currentSnowFrame.particles.reserve(aliveParticleCount());

    for (const SnowParticle& particle : particles) {
        if (particle.lifeSec <= 0.0f) {
            continue;
        }

        currentSnowFrame.particles.push_back(particle);
    }
}
