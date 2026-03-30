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
    auto& emitter = gCoordinator.GetComponent<SnowEmitter>(entity);
    auto& gridBox = gCoordinator.GetComponent<SnowEmitterGridBox>(entity);

    if (!gridBox.isValid()) return;

    // Generate grid spawn points in local space.
    std::vector<glm::vec3> gridPoints = generateGridSpawnPoints(gridBox);

    // Distribute spawns across grid points.
    for (int i = 0; i < gridPoints.size(); ++i) {
        const glm::vec3& gridLocalOffset = gridPoints[i % gridPoints.size()];
        emitter.localOffset = gridLocalOffset;
        spawnParticlesFromEmitter(entity, deltaTime);
    }
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
    SnowParticle& particle = particles[firstUnusedParticle()];

    particle.position = transform.pos + (transform.rot * emitter.localOffset);
    particle.lifeSec = emitter.particleLifetimeSec;
    particle.size = std::max(emitter.particleSize, 0.05f);
    particle.color = emitter.color;

    // Apply template-specific overrides for certain presets.
    if (emitter.preset == SnowEmitterPreset::SnowCannon) {
        glm::quat extraRotationY_180deg = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 1, 0));
        glm::quat extraRotationX_45deg = glm::angleAxis(glm::radians(-5.0f), glm::vec3(1, 0, 0));
        glm::vec3 forward = transform.rot * extraRotationY_180deg * extraRotationX_45deg * glm::vec3(0, 0, 1);

        float spread = 0.08f;
        glm::vec3 jitter = transform.rot * glm::vec3(
            math::random::RandomFloat(-spread, spread),
            math::random::RandomFloat(-spread, spread),
            0.0f
        );
        float muzzleVelocity = 45.0f;
        particle.velocity = (forward + jitter) * muzzleVelocity;
    }
    else if (emitter.preset == SnowEmitterPreset::Nitro) {
        const glm::vec3 forward = transform.rot * glm::vec3(0, 0, 1);
        const float speed = 3.5f;

        particle.velocity = (-forward * speed);
    }
    else if (emitter.preset == SnowEmitterPreset::AvalancheFront) {
        const float jitterX = math::random::RandomFloat(-jitterAmount, jitterAmount);
        const float jitterZ = math::random::RandomFloat(-jitterAmount, jitterAmount);
        const float upVelocity = math::random::RandomFloat(upVelocityMin, upVelocityMax);
        const float driftVelocityX = math::random::RandomFloat(driftVelocityMin, driftVelocityMax);
        const float driftVelocityZ = math::random::RandomFloat(driftVelocityMin, driftVelocityMax);

        particle.position = transform.pos + (transform.rot * emitter.localOffset) + glm::vec3(jitterX, 0.0f, jitterZ);
        particle.velocity = transform.rot * glm::vec3(driftVelocityX, upVelocity, driftVelocityZ);
    }
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
    const int maxX = gridBox.gridResolution.x - 1;
    const int maxY = gridBox.gridResolution.y - 1;
    const int maxZ = gridBox.gridResolution.z - 1;

    // Generate grid cell centers in local space, origin at box center.
    for (int x = 0; x < gridBox.gridResolution.x; ++x) {
        for (int y = 0; y < gridBox.gridResolution.y; ++y) {
            for (int z = 0; z < gridBox.gridResolution.z; ++z) {
                bool includeCell = true;
                switch (gridBox.pattern) {
                case SnowEmitterGridPattern::All:
                    includeCell = true;
                    break;
                case SnowEmitterGridPattern::Checkerboard:
                    includeCell = ((x + y + z) % 2 == 0);
                    break;
                case SnowEmitterGridPattern::Border:
                    includeCell = (x == 0 || x == maxX || y == 0 || y == maxY || z == 0 || z == maxZ);
                    break;
                default:
                    includeCell = true;
                    break;
                }

                if (!includeCell) {
                    continue;
                }

                // Compute cell center.
                const glm::vec3 cellCenter = -halfBox + cellSize * glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f);
                points.push_back(gridBox.localOffset + cellCenter);
            }
        }
    }

    return points;
}
