#include "SnowBallisticSystem.hpp"
#include "utils/logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/projection.hpp>

#include <GLFW/glfw3.h>

extern Coordinator gCoordinator;

extern Entity playerVehicleEntity;


SnowBallisticSystem::SnowBallisticSystem(
    std::shared_ptr<RenderingSystem> renderingSystem,
    std::shared_ptr<VehicleControlSystem> vehicleControlSystem)
    : renderingSystem(renderingSystem),
    vehicleControlSystem(vehicleControlSystem)
{
}

/// =========== Update loop ===============

void SnowBallisticSystem::update(float dt) {
    const auto& vehicles = vehicleControlSystem->mEntities;

    for (auto const& cannon : mEntities) {
        auto& cTrans = gCoordinator.GetComponent<PhysxTransform>(cannon);
        auto& cData = gCoordinator.GetComponent<SnowCannon>(cannon);

        // 1. Update Oscillation
        cData.oscillationTimer += dt;

        // Speed: 1.0f (can be adjusted), Range: +/- 20 degrees
        float angle = glm::sin(cData.oscillationTimer * 1.5f) * glm::radians(20.0f);

        // Apply rotation on Y axis relative to initial orientation
        // Note: Assuming you store restRotation or rotate current local Y
        glm::quat extraRot = glm::angleAxis(angle, glm::vec3(0, 1, 0));
        cTrans.rot = cData.restRotation * extraRot; // Apply to base rotation
        glm::vec3 cPos = cTrans.pos;
        glm::vec3 cForward = cTrans.forward();

        for (auto const& vehicle : vehicles) {
            auto& vTrans = gCoordinator.GetComponent<PhysxTransform>(vehicle);
            auto& vComp = gCoordinator.GetComponent<VehicleComponent>(vehicle);
            glm::vec3 relPos = vTrans.pos - cPos;

            // 1. Quick sphere check (40m^2 = 1600)
            float d2 = glm::dot(relPos, relPos);
            if (d2 > 1600.f) continue;

            // 2. Front/Back check (Dot product)
            if (glm::dot(relPos, cForward) > 0) continue;

            // 3. Lateral stream check (10m^2 = 100)
            if (getDistSqToStream(vTrans.pos, cTrans) < 100.f) {
                vComp.inSnowStream = true;
            }
        }
    }
}

float SnowBallisticSystem::getDistSqToStream(const glm::vec3& p, const PhysxTransform& trans) {
    glm::vec3 dir = trans.forward();
    glm::vec3 v = p - trans.pos;
    float projection = glm::dot(v, dir);
    // Projection distance along the stream axis
    glm::vec3 closestPoint = trans.pos + (dir * projection);
    glm::vec3 distVec = p - closestPoint;
    return glm::dot(distVec, distVec);
}

/// =========== SETUP Functions ===============

void SnowBallisticSystem::init() {
    setupSnowCannonsFromPosAndRot();
}

void SnowBallisticSystem::setupSnowCannonsFromPosAndRot() {
    logger::info("SnowBallisticSystem: Starting batch setup of cannons...");

    for (const auto& cannonData : snowCannonsInitialPosAndRot)
    {
        setupSnowCannonEntity(cannonData.first, cannonData.second);
    }

    logger::info("SnowBallisticSystem: {} cannons initialized.", snowCannonsInitialPosAndRot.size());
}

void SnowBallisticSystem::setupSnowCannonEntity(glm::vec3 position, glm::quat rotation) {
    //PhysxTransform Component
    Entity snowCannonEntity = renderingSystem->createModelEntity("assets/obj/cannon/cannon.obj");
    auto& snowCannonTrans = gCoordinator.GetComponent<PhysxTransform>(snowCannonEntity);

    //Scale
    snowCannonTrans.scale = glm::vec3(4.0f);

    //Position
    glm::vec3 offset = glm::vec3(0.f, 2.67f, 0.f);
    snowCannonTrans.pos = position + offset;

    //Rotation
    //glm::quat extraRotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 1, 0));
    snowCannonTrans.rot = rotation;

    //SnowCannon Component
    SnowCannon cannonComp;
    cannonComp.restRotation = snowCannonTrans.rot;
    gCoordinator.AddComponent(snowCannonEntity, cannonComp);

    SnowEmitter emitter;
    emitter.enabled = true;
    emitter.preset = SnowEmitterPreset::SnowCannon;
    emitter.spawnRate = 50.f;
    emitter.particleLifetimeSec = 1.0f;
    emitter.particleSize = 2.5f;
    emitter.color = {0.92f, 0.96f, 1.0f};
    gCoordinator.AddComponent(snowCannonEntity, emitter);

    SnowEmitterGridBox grid;
    grid.enabled = true;
    grid.localBoxSize = glm::vec3(2.5f, 2.5f, 0.1f);
    grid.gridResolution = glm::ivec3(2, 2, 1);
    grid.localOffset = glm::vec3(0.0f, 0.0f, 1.5f);
    grid.pattern = SnowEmitterGridPattern::All;
    gCoordinator.AddComponent(snowCannonEntity, grid);
}
