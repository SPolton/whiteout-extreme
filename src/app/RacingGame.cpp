#include "RacingGame.hpp"


//#include "components/CameraComponent.h"
#include "components/Model.h"
#include "components/Renderable.h"
#include "components/Transform.h"
#include "components/VehicleComponent.h"
#include "ecs/Coordinator.hpp"

#include "input/Inputmanager.hpp"

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

    // Create the player vehicle entity with physics components
    playerVehicleEntity = physicsSystem->createVehicleEntity();
    gCoordinator.GetComponent<VehicleComponent>(playerVehicleEntity).playerID = 0;
    gCoordinator.AddComponent(playerVehicleEntity, Renderable{
        renderingSystem->getCubeRenderable().geometry,
        renderingSystem->getCubeRenderable().cpuData,
        renderingSystem->getCubeRenderable().shader,
        renderingSystem->vehicleTexture.get() // Use the same texture as the spheres for simplicity
        });

    // 4.You can modify Component Data for entities
    
    // Position Earth at the origin
    gCoordinator.GetComponent<PhysxTransform>(Earth).pos = glm::vec3(0.0f, 0.0f, 0.0f);
    gCoordinator.GetComponent<PhysxTransform>(Earth).scale = glm::vec3(1.0f);
    // Earth keeps the default earth texture
    
    // Position Mars to the side
    gCoordinator.GetComponent<PhysxTransform>(Mars).pos = glm::vec3(3.0f, 10.0f, 0.0f);
    gCoordinator.GetComponent<PhysxTransform>(Mars).scale = glm::vec3(0.5f); // Smaller than Earth
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

    // get inputs
    inputManager = renderingSystem->getInputManager();
}


/// Main game loop
void RacingGame::run()
{
    bool addedRigidBodyToMars = false;

    while (!renderingSystem->shouldClose())
    {
        // check for keyboard inputs first
        // triggers pause menu (only allow keyboard input if game is paused or we are in game)
        if (inputManager->isKeyPressedOnce(GLFW_KEY_P) && (gameState == 2 || gameState == 1)) {
            togglePause();
        }
        // triggers main menu (do not allow keyboard input to navigate to main menu while in game)
        else if (inputManager->isKeyPressedOnce(GLFW_KEY_M) && gameState != 1) {
            toggleMainMenu();
        }

        // if in game
        if (gameState == 1) {
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
            textSystem->beginText();

            textSystem->loadFont("arial.ttf", 48);

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
        else if (gameState == 2) {
            renderPauseMenu();
        }
        else if (gameState == 0) {
            renderMainMenu();
        }
    }
    logger::info("Shutting down systems...");
    renderingSystem->cleanup();
}


// Pause Menu
//==================================================================================================================//

void RacingGame::togglePause() {
    // update game status
    if (gameState == 1) {
        gameState = 2; // pause game
    }
    else if (gameState == 2) {
        gameState = 1; // resume game
    }

    if (gameState == 2) {
        logger::info("Game is paused...");
        renderPauseMenu();
    }
    else {
        logger::info("Game resumed...");
    }
}

void RacingGame::renderPauseMenu() {
    // Clear buffers
    glClearColor(0.6f, 0.8f, 1.0f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // when in a menu, check for cursor position to highlight "buttons"
    // get cursor position
    glm::dvec2 cursorPos = inputManager->CursorPosition();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    textSystem->renderText("PAUSE MENU", { 460.f, 1100.f, 0.75f }, { 0.f, 0.f, 0.55f });

    textSystem->loadFont("SNPro-SemiBold.ttf", 85);

    // default color for the buttons
    glm::vec3 defaultColor = { 0.f, 0.f, 0.6f };

    // set the buttons to the default color (used when not hovered upon)
    glm::vec3 resumeColor = defaultColor;
    glm::vec3 quitColor = defaultColor;

    // check keyboard input
    if (inputManager->isKeyPressedOnce(GLFW_KEY_P)) {
        gameState = 1; // resume game
    }
    else if (inputManager->isKeyPressedOnce(GLFW_KEY_M)) {
        gameState = 0; // return to main menu
    }

    // check if mouse is hovered over the "Resume" button
    if (cursorPos.x > 325.f && cursorPos.x < 460.f) {
        if (cursorPos.y > 200.f && cursorPos.y < 230.f) {
            // if it is, highlight in red
            resumeColor = { 0.8f, 0.f, 0.f };

            // and check if the user clicks on the mouse while over the "Resume" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // if they do, toggle pause
                togglePause();
            }
        }
    }

    // check if mouse is hovered over the "Quit" button
    if (cursorPos.x > 205.f && cursorPos.x < 595.f) {
        if (cursorPos.y > 260.f && cursorPos.y < 290.f) {
            // if it is, highlight in red
            quitColor = { 0.8f, 0.f, 0.f };

            // and check if the user clicks on the mouse while over the "Quit" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // and toggle to show the main menu
                toggleMainMenu();
            }
        }
    }

    // render the text with the proper color assigned
    textSystem->renderText("Resume", { 590.f, 900.f, 0.75f }, resumeColor);
    textSystem->renderText("Quit (Exit to Main Menu)", { 370.f, 750.f, 0.75f }, quitColor);

    textSystem->endText();

    // swap buffer
    renderingSystem->endFrame();
}

// Main Menu
//==================================================================================================================//

void RacingGame::toggleMainMenu() {
    // update game status
    if (gameState == 0) {
        gameState = 1; // start game
    }
    else if (gameState == 2) {
        gameState = 0; // go to main menu form pause menu
    }

    if (gameState == 0) {
        logger::info("On home page...");
        renderMainMenu();
    }
    else {
        logger::info("In game...");
    }
}

void RacingGame::renderMainMenu() {
    // Clear buffers
    glClearColor(0.6f, 0.8f, 1.0f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // when in a menu, check for cursor position to highlight "buttons"
    // get cursor position
    glm::dvec2 cursorPos = inputManager->CursorPosition();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    textSystem->renderText("Whiteout Extreme", { 300.f, 1100.f, 0.75f }, { 0.f, 0.f, 0.55f });

    // default color for the "Start" button
    glm::vec3 defaultColor = { 0.f, 0.f, 0.6f };

    // set the "Start" button to the default color (used when not hovered upon)
    glm::vec3 startColor = defaultColor;

    // check if mouse is hovered over the "Start" button
    if (cursorPos.x > 320.f && cursorPos.x < 460.f) {
        if (cursorPos.y > 400.f && cursorPos.y < 435.f) {
            // if it is, highlight in red
            startColor = { 0.8f, 0.f, 0.f };

            // and check if the user clicks on the mouse while over the "Start" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // if they do, toggle to NOT show the main menu
                toggleMainMenu();
            }
        }
    }

    // render the text with the proper color assigned
    textSystem->renderText("Start", { 585.f, 400.f, 0.75f }, startColor);

    textSystem->endText();

    // swap buffer
    renderingSystem->endFrame();
}
