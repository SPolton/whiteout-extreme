#include "SnowBallisticSystem.hpp"
#include "utils/logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <GLFW/glfw3.h>

extern Coordinator gCoordinator;

SnowBallisticSystem::SnowBallisticSystem(
    std::shared_ptr<RenderingSystem> renderingSystem,
    std::shared_ptr<VehicleControlSystem> vehicleControlSystem)
    : renderingSystem(renderingSystem),
    vehicleControlSystem(vehicleControlSystem)
{
}

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
    Entity snowCannonEnity = renderingSystem->createModelEntity("assets/obj/cannon/cannon.obj");
    auto& snowCannonTrans = gCoordinator.GetComponent<PhysxTransform>(snowCannonEnity);

    //Scale
    snowCannonTrans.scale = glm::vec3(4.0f);

    //Position
    glm::vec3 offset = glm::vec3(0.f, 2.67f, 0.f);
    snowCannonTrans.pos = position + offset;

    //Rotation
    //glm::quat extraRotation = glm::angleAxis(glm::radians(30.0f), glm::vec3(0, 1, 0));
    snowCannonTrans.rot = rotation;

    //SnowCannon Component
    SnowCannon cannonComp;
    cannonComp.restRotation = snowCannonTrans.rot;
    gCoordinator.AddComponent(snowCannonEnity, cannonComp);
}
