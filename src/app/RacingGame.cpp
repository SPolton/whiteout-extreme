

#include "RacingGame.hpp"
#include "ecs/Coordinator.hpp"
//#include "components/CameraComponent.h"
#include "components/Renderable.h"
#include "components/Transform.h"
#include "components/VehicleComponent.h"

//ECS global coordinator
Coordinator gCoordinator;


RacingGame::RacingGame()
{
    ///---- START OF ECS SETUP ----///
    // 0.Global ECS Coordinator Initialization
    gCoordinator.Init();

    // 1.Register Components
    //gCoordinator.RegisterComponent<CameraComponent>();
    gCoordinator.RegisterComponent<Renderable>();
    gCoordinator.RegisterComponent<PhysxTransform>();
    gCoordinator.RegisterComponent<RigidBody>();
    gCoordinator.RegisterComponent<VehicleComponent>();

    // 2.Create Systems and Set Signatures
    // Rendering System signature is made of Renderable + Transform components
    renderingSystem = gCoordinator.RegisterSystem<RenderingSystem>();
    {
        Signature signature;
        signature.set(gCoordinator.GetComponentType<Renderable>());
        signature.set(gCoordinator.GetComponentType<PhysxTransform>());
        gCoordinator.SetSystemSignature<RenderingSystem>(signature);
    }
    physicsSystem = gCoordinator.RegisterSystem<PhysicsSystem>();
    {
        Signature signature;
        signature.set(gCoordinator.GetComponentType<PhysxTransform>());
        signature.set(gCoordinator.GetComponentType<RigidBody>());

        gCoordinator.SetSystemSignature<PhysicsSystem>(signature);
    }

    physicsSystem->init();
    physicsSystem->spawnBoxPyramid(10, 0.5f, renderingSystem->getCubeRenderable());

    // 3.Create Entities and add Components to them:
    // Create a sphere entity for Earth
    Earth = renderingSystem->createSphereEntity();
    // Create a second sphere entity for Mars
    Mars = renderingSystem->createSphereEntity();
    // Note:
    // The createSphereEntity() method calls:
    //      - gCoordinator.AddComponent(sphere,PhysxTransform{...});
    //      - gCoordinator.AddComponent(sphere,Renderable{...});
    // Same components signature as RenderingSystem, so will be added to that system's entity list.

    // Create the player vehicle entity with physics components
    playerVehicleEntity = physicsSystem->createVehicleEntity();
    gCoordinator.AddComponent(playerVehicleEntity, Renderable{
        renderingSystem->getCubeRenderable().geometry,
        renderingSystem->getCubeRenderable().cpuData,
        renderingSystem->getCubeRenderable().shader,
        renderingSystem->vehicleTexture.get() // Use the same texture as the spheres for simplicity
        });

    // 4.You can modify Component Data for entities
    gCoordinator.GetComponent<PhysxTransform>(Mars).pos = glm::vec3(-1.3f, 0.7f, -0.4f); // Move Mars slightly
    gCoordinator.GetComponent<PhysxTransform>(Mars).scale = glm::vec3(0.3f); // Scale down Mars
    gCoordinator.GetComponent<PhysxTransform>(Mars).rot = glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)); // Rotate Mars
    gCoordinator.GetComponent<Renderable>(Mars).texture = renderingSystem->texture2.get(); // Mars texture

    ///---- END OF ECS SETUP ----///

    textSystem = std::make_unique<Text>();

    textSystem->setProjection(1440.0f, 1440.0f);
}


/// Main game loop
void RacingGame::run()
{
    bool addedRigidBodyToMars = false;

    while (!renderingSystem->shouldClose())
    {
        gameTime.update();

        // 5. You can also add/remove components at runtime to change entity behavior
        // After 20 seconds, add a RigidBody component to sphere2 (Mars) to make it fall
        if (gameTime.currentTime >= 20.0 && !addedRigidBodyToMars) {
            gCoordinator.GetComponent<PhysxTransform>(Mars).pos = glm::vec3(0.f, 20.f, 0.f);
            gCoordinator.GetComponent<PhysxTransform>(Mars).scale = glm::vec3(1.f);

            gCoordinator.AddComponent(
                Mars,
                physicsSystem->createRigidBodyFromSphere(Mars)
            ); // This will add the Mars entity to the PhysicsSystem's entity list and it will start falling due to gravity
            addedRigidBodyToMars = true;
            logger::info("Added RigidBody component to Entity Mars at t = {} seconds", gameTime.tF());
        }

        // Physics System Loop, adaptive based on performance
        int maxPhysicsSteps = gameTime.maxPhysicsSteps();
        int physicsSteps = 0;
        while (gameTime.accumulator >= gameTime.dt && physicsSteps < maxPhysicsSteps) {
            if (gameTime.frameCount < 600) {
                break; // Skip the first frames to avoid slow startup
            }
            physicsSystem->update(gameTime.dtF());
            gameTime.physicsUpdate();
            physicsSteps++;
        }
        
        // Discard excess time when running slow to prevent spiral of death
        if (physicsSteps >= maxPhysicsSteps) {
            gameTime.discardExcessTime();
        }

        renderingSystem->update(gameTime.fpsF());

        // If entity exists, update camera target to follow the player vehicle
        // We don't assume anymore that it's in the first position of the entity list, so we directly access it by its Entity ID
        if (gCoordinator.HasComponent<PhysxTransform>(playerVehicleEntity)) {
            renderingSystem->updateCameraTarget(gCoordinator.GetComponent<PhysxTransform>(playerVehicleEntity).pos);
        }

        renderingSystem->updateUI();

        // Must be called after renderer update, but before text rendering
        textSystem->beginText();

        textSystem->renderText("Hello!",
            { 100.f, 1200.f, 1.f }, { 0.5f, 0.8f, 0.2f });

        textSystem->renderText(
            "Rendered Frames: " + std::to_string(gameTime.frameCount),
            { 100.f, 1150.f, 0.75f }, { 0.2f, 0.5f, 0.8f });

        textSystem->renderText(
            "Physics Frames: " + std::to_string(gameTime.physicsFrameCount),
            { 100.f, 1100.f, 0.75f }, { 0.5f, 0.2f, 0.8f });

        textSystem->renderText(
            "Game FPS: " + std::to_string(1.0f / gameTime.fpsF()),
            { 100.f, 1050.f, 0.75f }, { 0.8f, 0.8f, 0.2f });

        textSystem->endText();

        // Must be called last
        renderingSystem->endFrame();
    }
    logger::info("Shutting down systems...");
    renderingSystem->cleanup();
}
