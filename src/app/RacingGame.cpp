#include "RacingGame.hpp"


//#include "components/CameraComponent.h"
#include "components/Model.h"
#include "components/Physics.hpp"
#include "components/Renderable.h"
#include "components/Transform.h"
#include "components/VehicleComponent.h"
#include "ecs/Coordinator.hpp"

//ECS global coordinator
Coordinator gCoordinator;
Entity playerVehicleEntity;

RacingGame::RacingGame()
{
    ///---- Input Manager and Window ----/// 
    if (!glfwInit()) {
        logger::error("GLFW Init Failed");
        return;
    }

    inputManager = std::make_shared<InputManager>();
    window = std::make_shared<Window>(inputManager, 1200, 800, "Whiteout Extreme");
    window->makeContextCurrent();

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        logger::error("GLAD Init Failed");
        return;
    }

    ///---- Imgui Wrapper and Panel ----/// 
    imguiWrapper = std::make_unique<ImGuiWrapper>();
    if (!imguiWrapper->init(window->getGLFWwindow())) {
        logger::error("ImGui Init Failed");
    }
    imguiPanel = std::make_unique<ImGuiPanel>();
    logger::info("ImGui initialized");

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
    gCoordinator.RegisterComponent<AvalancheComponent>();

    // 2.Create Systems and Set Signatures
    // RENDERING SYSTEM: Requires Transform AND <Renderable OR ModelRenderable>
    renderingSystem = gCoordinator.RegisterSystem<RenderingSystem>(inputManager);
    {
        Signature signature1;
        signature1.set(gCoordinator.GetComponentType<PhysxTransform>());
        signature1.set(gCoordinator.GetComponentType<Renderable>());
        gCoordinator.SetSystemSignature<RenderingSystem>(signature1);

        Signature signature2;
        signature2.set(gCoordinator.GetComponentType<PhysxTransform>());
        signature2.set(gCoordinator.GetComponentType<ModelRenderable>());
        gCoordinator.SetSystemSignature<RenderingSystem>(signature2);
    }

    renderingSystem->vWidth = window->getWidth();
    renderingSystem->vHeight = window->getHeight();

    window->setCallbacks(inputManager);
    inputManager->setResizeCallback([this](int w, int h) {
        glViewport(0, 0, w, h);
        renderingSystem->vWidth = w;
        renderingSystem->vHeight = h;
        logger::info("Window resized to {}x{}", w, h);
        });

    inputManager->setMouseWheelCallback([this](double w, double h) {
        renderingSystem->onMouseWheelChange(w, h);
        });


    // PHYSICS SYSTEM : Requires Transform AND RigidBody
     physicsSystem = gCoordinator.RegisterSystem<PhysicsSystem>();
    {
        Signature signature;
        signature.set(gCoordinator.GetComponentType<PhysxTransform>());
        signature.set(gCoordinator.GetComponentType<RigidBody>());
        gCoordinator.SetSystemSignature<PhysicsSystem>(signature);
    }

    physicsSystem->spawnBoxPyramid(10, 0.5f, renderingSystem->getCubeRenderable("assets/textures/carbon_fiber.jpg"));

    // VEHICLE CONTROL SYSTEM
     vehicleControlSystem = gCoordinator.RegisterSystem<VehicleControlSystem>(
        inputManager,
        gCoordinator.GetSystem<RenderingSystem>(),
        gCoordinator.GetSystem<PhysicsSystem>()
    );
    {
        Signature signature;
        signature.set(gCoordinator.GetComponentType<VehicleComponent>());
        gCoordinator.SetSystemSignature<VehicleControlSystem>(signature);
    }

    // 3.Create Entities and add Components to them:
    
    // Create Skybox first (if texture is available)
    Skybox = renderingSystem->createSkyboxEntity("assets/textures/sky/snow_landscape.hdr");
    logger::info("Created Skybox entity");

    // Create Earth sphere entity
    Earth = renderingSystem->createSphereEntity("assets/textures/2k_earth_daymap.jpg");
    logger::info("Created Earth sphere entity");
    
    // Create Mars sphere entity
    Mars = renderingSystem->createSphereEntity("assets/textures/2k_mars.jpg");
    logger::info("Created Mars sphere entity");
    
    // Create Woody model entity (separate from Earth and Mars)
    WoodyModel = renderingSystem->createModelEntity("assets/obj/woody.obj");
    logger::info("Created Woody model entity");
    
    // Create Backpack model entity to test shading maps
    BackpackModel = renderingSystem->createModelEntity("assets/obj/backpack/backpack.obj");
    logger::info("Created Backpack model entity");

    // Create Map model entity
    MapModel = renderingSystem->createModelEntity("assets/obj/map/map.obj");

    // Adjust scale and position
    auto& mapTransform = gCoordinator.GetComponent<PhysxTransform>(MapModel);
    mapTransform.scale = glm::vec3(0.2f);
    mapTransform.pos = glm::vec3(0.f, -3.f, -50.f);
    mapTransform.rot = glm::vec3(0.f, 45.f, 0.f);

    // Add physics collision to the map entity
    gCoordinator.AddComponent(
        MapModel,
        physicsSystem->createRigidBodyFromMesh(MapModel)
    );

    logger::info("Created Map model entity with collision");

    // Note:
    // The createSphereEntity() method calls:
    //      - gCoordinator.AddComponent(sphere,PhysxTransform{...});
    //      - gCoordinator.AddComponent(sphere,Renderable{...});
    // Same components signature as RenderingSystem, so will be added to that system's entity list.

    // Create the player vehicle entity with physics components
    playerVehicleEntity = physicsSystem->createVehicleEntity();
    gCoordinator.GetComponent<VehicleComponent>(playerVehicleEntity).playerID = 0;

    // Load snowmobile model for the player vehicle
    Entity snowmobileVisual = renderingSystem->createModelEntity("assets/obj/snowmobile/snowmobile.obj");
    auto& snowmobileRenderable = gCoordinator.GetComponent<ModelRenderable>(snowmobileVisual);
    gCoordinator.AddComponent(playerVehicleEntity, snowmobileRenderable);
    gCoordinator.DestroyEntity(snowmobileVisual);

    // Fix rotation and scale
    auto& vehicleTransform = gCoordinator.GetComponent<PhysxTransform>(playerVehicleEntity);
    vehicleTransform.rot = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.f, 1.f, 0.f));
    vehicleTransform.scale = glm::vec3(1.0f);  // Uniform scale instead of stretched box scale

    logger::info("Loaded snowmobile model for player vehicle");

    // 4.You can modify Component Data for entities
    
    // Create the avalanche entity (appears far behind the starting position)
    Entity avalancheEntity = physicsSystem->createAvalancheEntity(glm::vec3(0.f, 15.f, -200.f), 15.0f);
    
    // Add rendering to the avalanche
    gCoordinator.AddComponent(
        avalancheEntity,
        renderingSystem->getCubeRenderable("assets/textures/snowball.png")
    );
    logger::info("Avalanche entity created");

    // Position Earth at the origin
    gCoordinator.GetComponent<PhysxTransform>(Earth).pos = glm::vec3(0.0f, 0.0f, 0.0f);
    gCoordinator.GetComponent<PhysxTransform>(Earth).scale = glm::vec3(1.0f);
    // Earth keeps the default earth texture
    
    // Position Mars to the side
    gCoordinator.GetComponent<PhysxTransform>(Mars).pos = glm::vec3(1.3f, 0.7f, 0.7f); // Move Mars slightly
    gCoordinator.GetComponent<PhysxTransform>(Mars).scale = glm::vec3(0.4f); // Scale down Mars
    gCoordinator.GetComponent<PhysxTransform>(Mars).rot = glm::angleAxis(glm::radians(23.5f), glm::vec3(0.f, 0.f, 1.f)); // Tilt Mars
    
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

    menus = std::make_unique<GameMenus>(textSystem.get(), inputManager.get(), gameState);
}


/// Main game loop
void RacingGame::run()
{
    bool addedRigidBodyToMars = false;
    bool MarsIsBack = false;

    while (!window->shouldClose())
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
                    gCoordinator.RemoveComponent<RigidBody>(Mars); // Remove physics from Mars
                }
            }

            // Vehicle control system Loop - process player inputs and update vehicle state before physics simulation
            //vehicleControlSystem->update(gameTime.dtF());

            // Physics System Loop, adaptive based on performance
            int maxPhysicsSteps = gameTime.maxPhysicsSteps();
            int physicsSteps = 0;
            while (gameTime.accumulator >= gameTime.dt && physicsSteps < maxPhysicsSteps) {
                if (gameTime.frameCount < static_cast<unsigned>(300) && gameTime.physicsFrameCount > static_cast<unsigned>(maxPhysicsSteps)) {
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

            // Process Escape key input to close window
            if (inputManager->isKeyPressedOnce(GLFW_KEY_ESCAPE))
                glfwSetWindowShouldClose(window->getGLFWwindow(), true);

            // Process F key input to toggle camera, and IJKLUO to move the free camera
            renderingSystem->update(gameTime.fpsF());

            // Update values and sync imgui parameters
            this->updateImGui();


            // If entity exists, update camera target to follow the player vehicle
            // We don't assume anymore that it's in the first position of the entity list, so we directly access it by its Entity ID
            if (gCoordinator.HasComponent<PhysxTransform>(playerVehicleEntity)) {
                glm::vec3 targetPos = gCoordinator.GetComponent<PhysxTransform>(playerVehicleEntity).pos;
                targetPos.y += 2.f;
                renderingSystem->updateCameraTarget(targetPos);
            }

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
            this->endFrame();
        }
        else if (gameState == GameState::MainMenu) {
            // render UI for main menu, take note of the action taken
            MenuAction actionCursor = menus->renderMainMenu();

            // if "Start" is pressed, go in the game
            if (actionButtons == MenuAction::StartGame || actionCursor == MenuAction::StartGame) {
                gameState = GameState::InGame;
            }

            // swap buffer
            this->endFrame();
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
            this->endFrame();
        }
        else if (gameState == GameState::GameOver) {
            // render UI for race finished, take note of the action taken
            MenuAction actionCursor = menus->renderGameOver();

            // if "Return to main menu" is pressed, return to the main menu
            if (actionButtons == MenuAction::GoToMainMenu || actionCursor == MenuAction::GoToMainMenu) {
                gameState = GameState::MainMenu;
            }

            // swap buffer
            this->endFrame();
        }
    }
    logger::info("Shutting down systems...");
}

void RacingGame::updateImGui() {
    glm::vec3 bgColor = imguiPanel->getBackgroundColor();
    glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);

    // Set viewport
    glViewport(0, 0, window->getWidth(), window->getHeight());

    // Apply wireframe mode if enabled
    if (imguiPanel->showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Update camera stats for UI
    imguiPanel->cameraStats = renderingSystem->getActiveCameraStats();
    imguiPanel->cameraStats.aspect = static_cast<float>(window->getWidth()) / static_cast<float>(window->getHeight());

    // Update UI
    imguiWrapper->beginFrame();
    imguiPanel->update();
    imguiWrapper->renderFPS();
    this->syncImgui();
    imguiPanel->cameraStats = renderingSystem->getActiveCameraStats();
    imguiWrapper->endFrame();
};

void RacingGame::syncImgui() {
    renderingSystem->camSpeed = imguiPanel->camSpeed;
    renderingSystem->camZoomSpeed = imguiPanel->camZoomSpeed;
    //renderingSystem->wireframeMode = imguiPanel->showWireframe;
}

void RacingGame::endFrame() {
    window->swapBuffers();
    glfwPollEvents();
}
