#include "RacingGame.hpp"


//#include "components/CameraComponent.h"
#include "components/Model.h"
#include "components/Renderable.h"
#include "components/Transform.h"
#include "components/VehicleComponent.h"
#include "ecs/Coordinator.hpp"

//ECS global coordinator
Coordinator gCoordinator;
std::shared_ptr<RenderingSystem> renderingSystem;
std::shared_ptr<PhysicsSystem> physicsSystem;
Entity playerVehicleEntity;

RacingGame::RacingGame()
{
    ///---- START OF ECS SETUP ----///
    // 0.Global ECS Coordinator Initialization
    gCoordinator.Init();

    // 1.Register Components
    //gCoordinator.RegisterComponent<CameraComponent>();
    gCoordinator.RegisterComponent<Renderable>();
    gCoordinator.RegisterComponent<ModelRenderable>();
    gCoordinator.RegisterComponent<PhysxTransform>();
    gCoordinator.RegisterComponent<RigidBody>();
    gCoordinator.RegisterComponent<VehicleComponent>();

    // 2.Create Systems and Set Signatures
    // Rendering System signature: Only requires Transform component
    // Entities can have either Renderable OR ModelRenderable (checked at render time)
    renderingSystem = gCoordinator.RegisterSystem<RenderingSystem>();
    {
        Signature signature;
        signature.set(gCoordinator.GetComponentType<PhysxTransform>());
        gCoordinator.SetSystemSignature<RenderingSystem>(signature);
    }

    // PHYSICS SYSTEM
    physicsSystem = gCoordinator.RegisterSystem<PhysicsSystem>();
    {
        Signature signature;
        signature.set(gCoordinator.GetComponentType<PhysxTransform>());
        signature.set(gCoordinator.GetComponentType<RigidBody>());
        gCoordinator.SetSystemSignature<PhysicsSystem>(signature);
    }

    physicsSystem->init();
    physicsSystem->spawnBoxPyramid(10, 0.5f, renderingSystem->getCubeRenderable());

    // VEHICLE CONTROL SYSTEM
    vehicleControlSystem = gCoordinator.RegisterSystem<VehicleControlSystem>();
    {
        Signature signature;
        signature.set(gCoordinator.GetComponentType<VehicleComponent>());
        gCoordinator.SetSystemSignature<VehicleControlSystem>(signature);
    }
    // We need to set the input manager for the vehicle control system so it can read player inputs
    // Borrowed from the rendering system since it creates and owns the input manager
    vehicleControlSystem->SetInputManager(renderingSystem->getInputManager());

    // 3.Create Entities and add Components to them:
    
    // Create Skybox first (if texture is available)
    Entity Skybox = renderingSystem->createSkyboxEntity();
    logger::info("Created Skybox entity");
    
    // Create the player vehicle entity with physics components
    playerVehicleEntity = physicsSystem->createVehicleEntity();
    gCoordinator.GetComponent<VehicleComponent>(playerVehicleEntity).playerID = 0;
    gCoordinator.AddComponent(playerVehicleEntity, Renderable{
        renderingSystem->getCubeRenderable().geometry,
        renderingSystem->getCubeRenderable().cpuData,
        renderingSystem->getCubeRenderable().shader,
        renderingSystem->vehicleTexture.get() // Use the same texture as the spheres for simplicity
        });

    // Create Earth sphere entity
    Earth = renderingSystem->createSphereEntity();
    logger::info("Created Earth sphere entity");
    
    // Create Mars sphere entity
    Mars = renderingSystem->createSphereEntity();
    logger::info("Created Mars sphere entity");
    
    // Create Woody model entity (separate from Earth and Mars)
    WoodyModel = renderingSystem->createModelEntity("assets/obj/woody.obj");
    logger::info("Created Woody model entity");
    
    // Create Backpack model entity to test shading maps
    BackpackModel = renderingSystem->createModelEntity("assets/obj/backpack/backpack.obj");
    logger::info("Created Backpack model entity");

    // Note:
    // The createSphereEntity() method calls:
    //      - gCoordinator.AddComponent(sphere,PhysxTransform{...});
    //      - gCoordinator.AddComponent(sphere,Renderable{...});
    // Same components signature as RenderingSystem, so will be added to that system's entity list.

    // 4.You can modify Component Data for entities
    
    // Position Earth at the origin
    gCoordinator.GetComponent<PhysxTransform>(Earth).pos = glm::vec3(0.0f, 0.0f, 0.0f);
    gCoordinator.GetComponent<PhysxTransform>(Earth).scale = glm::vec3(1.0f);
    // Earth keeps the default earth texture
    
    // Position Mars to the side
    gCoordinator.GetComponent<PhysxTransform>(Mars).pos = glm::vec3(1.3f, 0.7f, -0.7f); // Move Mars slightly
    gCoordinator.GetComponent<PhysxTransform>(Mars).scale = glm::vec3(0.4f); // Scale down Mars
    gCoordinator.GetComponent<PhysxTransform>(Mars).rot = glm::angleAxis(glm::radians(23.5f), glm::vec3(0.f, 0.f, 1.f)); // Tilt Mars
    gCoordinator.GetComponent<Renderable>(Mars).texture = renderingSystem->texture2.get(); // Mars texture
    
    // Position Woody model on the other side
    if (WoodyModel != 0) { // Check if model was successfully created
        gCoordinator.GetComponent<PhysxTransform>(WoodyModel).pos = glm::vec3(-3.0f, 0.0f, -2.0f);
        gCoordinator.GetComponent<PhysxTransform>(WoodyModel).scale = glm::vec3(0.01f); // Scale down (This OBJ model is large)
        // Model uses its own embedded textures
    }
    
    // Position Backpack model to test shading maps
    if (BackpackModel != 0) { // Check if model was successfully created
        gCoordinator.GetComponent<PhysxTransform>(BackpackModel).pos = glm::vec3(3.0f, 0.0f, -10.0f);
        // Model will use its material/texture maps from the .mtl file
    }

    ///---- END OF ECS SETUP ----///

    textSystem = std::make_unique<Text>();

    textSystem->setProjection(1440.0f, 1440.0f);

    menus = std::make_unique<GameMenus>(textSystem.get(), renderingSystem->getInputManager().get(), gameState);
}


/// Main game loop
void RacingGame::run()
{
    bool addedRigidBodyToMars = false;
    bool MarsIsBack = false;

    while (!renderingSystem->shouldClose())
    {
        // keep checking which input system we are using
        menus->checkInputSystem();

        // keep taking inputs in case pause menu is called
        MenuAction actionButtons = menus->pollInputs();

        // check for entering game
        if (actionButtons == MenuAction::StartGame || actionButtons == MenuAction::ResumeGame) {
            gameState = GameState::InGame;
        }

        // if in game
        if (gameState == GameState::InGame) {
            gameTime.update();

            // 5. You can also add/remove components at runtime to change entity behavior
            // After 20 seconds, add a RigidBody component to sphere2 (Mars) to make it fall
            if (gameTime.currentTime >= 15.0 && !addedRigidBodyToMars) {
                gCoordinator.GetComponent<PhysxTransform>(Mars).pos = glm::vec3(5.f, 10.f, 1.f);
                gCoordinator.GetComponent<PhysxTransform>(Mars).scale = glm::vec3(1.f);

            gCoordinator.AddComponent(
                Mars,
                physicsSystem->createRigidBodyFromSphere(Mars)
            ); // This will add the Mars entity to the PhysicsSystem's entity list and it will start falling due to gravity
            addedRigidBodyToMars = true;
            logger::info("Added RigidBody component to Entity Mars at t = {} seconds", gameTime.tF());
        }

        if(addedRigidBodyToMars && !MarsIsBack) {
            // Check if Mars position is close enough to Earth
            glm::vec3 earthPos = gCoordinator.GetComponent<PhysxTransform>(Earth).pos;
            glm::vec3 marsPos = gCoordinator.GetComponent<PhysxTransform>(Mars).pos;
            float distance = glm::length(earthPos - marsPos);
            if (distance < 2.5f) { // If Mars is close enough to Earth
                MarsIsBack = true;
                gCoordinator.GetComponent<PhysxTransform>(Mars).pos = glm::vec3(1.3f, 0.7f, -0.7f); // Move Mars slightly
                gCoordinator.GetComponent<PhysxTransform>(Mars).scale = glm::vec3(0.4f); // Scale down Mars
                gCoordinator.GetComponent<PhysxTransform>(Mars).rot = glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)); // Rotate Mars
                gCoordinator.GetComponent<Renderable>(Mars).texture = renderingSystem->texture2.get(); // Mars texture
                gCoordinator.RemoveComponent<RigidBody>(Mars); // Remove physics from Mars
            }
        }

        // Vehicle control system Loop - process player inputs and update vehicle state before physics simulation
        //vehicleControlSystem->update(gameTime.dtF());

        // Physics System Loop, adaptive based on performance
        int maxPhysicsSteps = gameTime.maxPhysicsSteps();
        int physicsSteps = 0;
        while (gameTime.accumulator >= gameTime.dt && physicsSteps < maxPhysicsSteps) {
            if (gameTime.frameCount < 300 && gameTime.physicsFrameCount > maxPhysicsSteps) {
                break; // Skip the first frames to avoid slow startup
            }
            vehicleControlSystem->update(gameTime.dtF());
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
            glm::vec3 targetPos = gCoordinator.GetComponent<PhysxTransform>(playerVehicleEntity).pos;
            targetPos.y += 2.f;
            renderingSystem->updateCameraTarget(targetPos);
        }

            renderingSystem->updateUI();

            // Must be called after renderer update, but before text rendering
            // auto width = renderingSystem->getWindowWidth();
            // auto height = renderingSystem->getWindowHeight();
            // textSystem->setProjection(width, height);

            textSystem->beginText();

            textSystem->loadFont("arial.ttf", 48);

            float marginX = 30.f;
            float topY = static_cast<float>(1440) - 50.f;

            textSystem->renderText("Hello!",
                { marginX, topY, 1.f }, { 0.5f, 0.8f, 0.2f });

            textSystem->renderText(
                "Rendered Frames: " + std::to_string(gameTime.frameCount),
                { marginX, topY - 50.f, 0.75f }, { 0.2f, 0.5f, 0.8f });

            textSystem->renderText(
                "Physics Frames: " + std::to_string(gameTime.physicsFrameCount),
                { marginX, topY - 100.f, 0.75f }, { 0.5f, 0.2f, 0.8f });

            textSystem->renderText(
                "Game FPS: " + std::to_string(static_cast<int>(1.0f / gameTime.fpsF())),
                { marginX, topY - 150.f, 00.75f }, { 0.8f, 0.8f, 0.2f });

            float centerX = static_cast<float>(1440) / 2.0f;
            float centerY = static_cast<float>(1440) / 2.0f;
            textSystem->renderText("+", { centerX - 5.f, centerY - 5.f, 0.75f }, { 1.f, 1.f, 1.f });

            textSystem->renderText(
                "Move: Left Joystick / WASD",
                { centerX * 1.2f, topY - 50.f, 0.7f }, { 0.0f, 0.90f, 0.95f });
            textSystem->renderText(
                "Boost: Y button / SHIFT",
                { centerX * 1.2f, topY - 100.f, 0.7f }, { 0.0f, 0.90f, 0.95f });
            textSystem->renderText(
                "Snowball: X button / SPACE",
                { centerX * 1.2f, topY - 150.f, 0.7f }, { 0.0f, 0.90f, 0.95f });
            textSystem->renderText(
                "Pause: Start button / P",
                { centerX * 1.2f, topY - 200.f, 0.7f }, { 0.0f, 0.90f, 0.95f });

            if(MarsIsBack) {
            textSystem->renderText(
                "BRAVO ! Mars has reached Earth",
                { centerX * 0.15f, centerY * 0.4f - 50.f, 0.6f }, { 1.f, 0.35f, 0.15f });
            } else if (addedRigidBodyToMars){
            textSystem->renderText(
                "Mars is gone. Don't Panic !",
                { centerX * 0.15f, centerY * 0.4f, 0.6f }, { 1.f, 0.35f, 0.15f });
            textSystem->renderText(
                "MISSION: Bring Mars back to Earth",
                { centerX * 0.15f, centerY * 0.4f - 50.f, 0.6f }, { 1.f, 0.35f, 0.15f });
            textSystem->renderText(
                "TIP: Nudge Mars back to Earth using snowballs",
                { centerX * 0.15f, centerY * 0.4f - 100.f, 0.6f }, { 1.f, 0.8f, 0.2f });
            }
            
            textSystem->endText();

            // Must be called last
            renderingSystem->endFrame();
        }
        else if (gameState == GameState::MainMenu) {
            // render UI for main menu, take note of the action taken
            MenuAction actionCursor = menus->renderMainMenu();

            // if "Start" is pressed, go in the game
            if (actionButtons == MenuAction::StartGame || actionCursor == MenuAction::StartGame) {
                gameState = GameState::InGame;
            }

            // swap buffer
            renderingSystem->endFrame();
        }
        else if (gameState == GameState::Pause) {
            // render UI for pause menu, take note of the action taken
            MenuAction actionCursor = menus->renderPauseMenu();

            // if "Resume" is pressed, return to the game
            if (actionButtons == MenuAction::ResumeGame || actionCursor == MenuAction::ResumeGame) {
                gameState = GameState::InGame;
            }
            // if "Quit" is pressed, return to the main menu
            else if (actionButtons == MenuAction::GoToMainMenu || actionCursor == MenuAction::GoToMainMenu) {
                gameState = GameState::MainMenu;
            }

            // swap buffer
            renderingSystem->endFrame();
        }
        else if (gameState == GameState::GameOver) {
            // render UI for race finished, take note of the action taken
            MenuAction actionCursor = menus->renderGameOver();

            // if "Return to main menu" is pressed, return to the main menu
            if (actionButtons == MenuAction::GoToMainMenu || actionCursor == MenuAction::GoToMainMenu) {
                gameState = GameState::MainMenu;
            }

            // swap buffer
            renderingSystem->endFrame();
        }
    }
    logger::info("Shutting down systems...");
    renderingSystem->cleanup();
}
