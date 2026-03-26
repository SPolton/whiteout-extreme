#include "RacingGame.hpp"


//#include "components/CameraComponent.h"
#include "components/Model.h"
#include "components/Physics.hpp"
#include "components/Renderable.h"
#include "components/Transform.h"
#include "components/VehicleComponent.h"
#include "ecs/Coordinator.hpp"

// test FMOD initialization
#include <fmod.hpp>
#include <iostream>
#include <fmod_studio_common.h>
// test FMOD initialization

//ECS global coordinator
Coordinator gCoordinator;
Entity playerVehicleEntity;
Entity aiVehicleEntity1;
Entity aiVehicleEntity2;

RacingGame::RacingGame()
{
    ///---- Input Manager and Window ----/// 
    if (!glfwInit()) {
        logger::error("GLFW Init Failed");
        return;
    }

    audioManager = std::make_shared<AudioEngine>();

    inputManager = std::make_shared<InputManager>();
    window = std::make_shared<Window>(inputManager, 1080, 720, "Whiteout Extreme");
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
    gCoordinator.RegisterComponent<Racer>();
    gCoordinator.RegisterComponent<AI>();

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

    // physicsSystem->spawnBoxPyramid(10, 0.5f, renderingSystem->getCubeRenderable("assets/textures/carbon_fiber.jpg"));

    // VEHICLE CONTROL SYSTEM : Requires VehicleComponent (which itself contains a pointer to the PhysX vehicle instance, so we can update it based on input)
     vehicleControlSystem = gCoordinator.RegisterSystem<VehicleControlSystem>(
        inputManager,
        audioManager,
        gCoordinator.GetSystem<RenderingSystem>(),
        gCoordinator.GetSystem<PhysicsSystem>()
    );
    {
        Signature signature;
        signature.set(gCoordinator.GetComponentType<VehicleComponent>());
        gCoordinator.SetSystemSignature<VehicleControlSystem>(signature);
    }

    // RACING SYSTEM: Requires Transform AND Racer
    racingSystem = gCoordinator.RegisterSystem<RacingSystem>(
        gCoordinator.GetSystem<RenderingSystem>(),
        gCoordinator.GetSystem<PhysicsSystem>()
    );
    {
        Signature signature;
        signature.set(gCoordinator.GetComponentType<VehicleComponent>());
        signature.set(gCoordinator.GetComponentType<Racer>());
        signature.set(gCoordinator.GetComponentType<PhysxTransform>());
        gCoordinator.SetSystemSignature<RacingSystem>(signature);
    }


    // AI SYSTEM: Requires Transform AND VehicleComponent AND Racer AND AI
    aiSystem = gCoordinator.RegisterSystem<AISystem>(
        gCoordinator.GetSystem<RenderingSystem>(),
        gCoordinator.GetSystem<PhysicsSystem>()
    );
    {
        Signature signature;
        signature.set(gCoordinator.GetComponentType<AI>());
        signature.set(gCoordinator.GetComponentType<Racer>());
        signature.set(gCoordinator.GetComponentType<VehicleComponent>());
        signature.set(gCoordinator.GetComponentType<PhysxTransform>());
        gCoordinator.SetSystemSignature<AISystem>(signature);
    }

    // 3.Create Entities and add Components to them:

    // Create the player vehicle entity with physics components
    //physx::PxVec3 spawnPos(-730.0f, 670.4f, -400.0f);
    aiVehicleEntity1 = physicsSystem->createVehicleEntity("VehicleAI1", physx::PxVec3(20.f, 0.f, 0.f));
    gCoordinator.GetComponent<VehicleComponent>(aiVehicleEntity1).playerID = 1;

    aiVehicleEntity2 = physicsSystem->createVehicleEntity("VehicleAI2", physx::PxVec3(30.f, 0.f, 0.f));
    gCoordinator.GetComponent<VehicleComponent>(aiVehicleEntity2).playerID = 2;

    playerVehicleEntity = physicsSystem->createVehicleEntity("VehiclePlayer1", physx::PxVec3(10.0f, 0.f, 0.f));
    gCoordinator.GetComponent<VehicleComponent>(playerVehicleEntity).playerID = 0;

    
    // Create Skybox first (if texture is available)
    render::SphereConfig skyboxConfig;
    skyboxConfig.radius = 100.0f;
    skyboxConfig.slices = 32;
    skyboxConfig.stacks = 32;
    skyboxConfig.isSkybox = true;
    Skybox = renderingSystem->createSphereEntity("assets/textures/sky/snow_landscape.hdr", skyboxConfig);
    gCoordinator.GetComponent<PhysxTransform>(Skybox).scale = glm::vec3(3.f);
    logger::info("Created Skybox entity");

    // Create infinite ground plane with repeating snow texture
    render::PlaneConfig planeConfig;
    planeConfig.isInfinite = true;
    planeConfig.uvRepeat = 500.0f;
    planeConfig.textureWrapMode = GL_REPEAT;
    GroundPlane = renderingSystem->createPlaneEntity("assets/textures/snowball.png", planeConfig);
    logger::info("Created ground plane entity");

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
    // Load snowmobile model for the player and AI vehicles
    Entity snowmobileVisual = renderingSystem->createModelEntity("assets/obj/snowmobile/snowmobile.obj");
    auto& snowmobileRenderable = gCoordinator.GetComponent<ModelRenderable>(snowmobileVisual);
    snowmobileRenderable.visualOffsetPos = glm::vec3(0.0f, 0.0f, 1.5f);
    gCoordinator.AddComponent(playerVehicleEntity, snowmobileRenderable);
    gCoordinator.AddComponent(aiVehicleEntity1, snowmobileRenderable);
    gCoordinator.AddComponent(aiVehicleEntity2, snowmobileRenderable);
    gCoordinator.DestroyEntity(snowmobileVisual);

    // Fix rotation and scale
    auto& vehicleTransform = gCoordinator.GetComponent<PhysxTransform>(playerVehicleEntity);
    vehicleTransform.rot = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.f, 1.f, 0.f));
    vehicleTransform.scale = glm::vec3(1.5f);  // Doubled the vehicle size

    logger::info("Loaded snowmobile model for player vehicle");

    auto& aiVehicleTransform = gCoordinator.GetComponent<PhysxTransform>(aiVehicleEntity1);
    aiVehicleTransform.rot = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.f, 1.f, 0.f));
    aiVehicleTransform.scale = glm::vec3(1.5f);  // Uniform scale instead of stretched box scale

    auto& aiVehicleTransform2 = gCoordinator.GetComponent<PhysxTransform>(aiVehicleEntity2);
    aiVehicleTransform2.rot = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.f, 1.f, 0.f));
    aiVehicleTransform2.scale = glm::vec3(1.5f);  // Uniform scale instead of stretched box scale

    logger::info("Loaded snowmobile models for ai vehicles");

    gCoordinator.AddComponent(playerVehicleEntity, Racer{});

    gCoordinator.AddComponent(aiVehicleEntity1, Racer{});
    gCoordinator.AddComponent(aiVehicleEntity1, AI{});

    gCoordinator.AddComponent(aiVehicleEntity2, Racer{});
    gCoordinator.AddComponent(aiVehicleEntity2, AI{});

    // 4.You can modify Component Data for entities
    
    // Create the avalanche entity (appears far behind the starting position)
    AvalancheEntity = physicsSystem->createAvalancheEntity(glm::vec3(0.f, 15.f, -200.f), 15.0f);
    
    // Add rendering to the avalanche
    auto avCubeRender = renderingSystem->getCubeRenderable("assets/textures/snowball.png");
    avCubeRender.hasRollingTexture = true;
    gCoordinator.AddComponent(AvalancheEntity, avCubeRender);
    logger::info("Avalanche entity created");

    auto& Avalanche = gCoordinator.GetComponent<AvalancheComponent>(AvalancheEntity).instance;

    racingSystem->init(Avalanche);
    logger::info("Loaded gates and avalanche for the race");

    ///---- END OF ECS SETUP ----///

    textSystem = std::make_unique<Text>();

    textSystem->setProjection(1440.0f, 1440.0f);

    menus = std::make_unique<GameMenus>(textSystem.get(), inputManager.get(), audioManager.get(), window.get(), gameState);
    menus->init();

   // intiailize audio engine
    audioManager->init();
    // load the main menu game music
    audioManager->loadSound("assets/audio/game-music-loop-12.mp3", false, true, true);
    musicChannelID = audioManager->playSounds("assets/audio/game-music-loop-12.mp3", { 0,0,0 }, -8.0f);
    // load the in-game music
    audioManager->loadSound("assets/audio/in-game-music.mp3", false, true, true);
    inGameMusicChannelID = audioManager->playSounds("assets/audio/in-game-music.mp3", { 0,0,0 }, -15.0f);
    // load avalanche sound
    audioManager->loadSound("assets/audio/rock-avalanche-2.wav", false, true, true);
    avalancheChannelID = audioManager->playSounds("assets/audio/rock-avalanche-2.wav", { 0,0,0 }, -20.0f);

    // call functions that will load sounds this component will use
    menus->loadMenuSounds();
    vehicleControlSystem->loadVehicleSounds();

    // play ai racer sounds
    audioManager->loadSound("assets/audio/snowmobiles-1-trimmed.wav", false, true, true);
    aiEngineChannelID1 = audioManager->playSounds("assets/audio/snowmobiles-1-trimmed.wav", { 0,0,0 }, -15.0f);
    aiEngineChannelID2 = audioManager->playSounds("assets/audio/snowmobiles-1-trimmed.wav", { 0,0,0 }, -15.0f);
}

RacingGame::~RacingGame()
{
    logger::info("Shutting down systems...");
    
    // shut down audio engine
    audioManager->shutdown();
}

/// Main game loop
void RacingGame::run()
{
    gameTime.reset(glfwGetTime());

    while (!window->shouldClose())
    {
        audioManager->update();

        // Keep checking for controller inputs and if menu actions are triggered
        menus->checkInputSystem();
        MenuAction actionButtons = menus->pollInputs();

        if (gameState == GameState::InGame) {
            updateInGame();
        } else {
            updateInMenu(actionButtons);
        }

        endFrame();
    }
}

// ----- State handlers ----- //

void RacingGame::updateInGame()
{
    gameTime.update(glfwGetTime());

    // Setup in-game audio channels
    audioManager->pauseChannel(musicChannelID);
    audioManager->resumeChannel(inGameMusicChannelID);
    audioManager->resumeChannel(avalancheChannelID);

    // Run fixed-step physics and game systems
    updatePhysicsAndGameplayLoop();

    // Get player state for audio and camera updates
    glm::vec3 playerPos = gCoordinator.GetComponent<PhysxTransform>(playerVehicleEntity).pos;
    float speed = glm::length(gCoordinator.GetComponent<RigidBody>(playerVehicleEntity).linearVelocity);

    // Update spatial audio (avalanche distance, engine speed gating, AI sounds)
    updateInGameAudioState(speed);

    // Check for quit input
    if (inputManager->isKeyPressedOnce(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window->getGLFWwindow(), true);
    }

    // Update camera to follow player
    updateInGameCameraTarget(speed);

    // Render scene and UI
    renderingSystem->update(gameTime.fpsF());
    updateImGui();
    renderInGameHUD();
}

void RacingGame::updateInMenu(MenuAction actionButtons)
{
    // Reset to prevent big delta spike when returning to gameplay
    gameTime.updatePause(glfwGetTime());

    handleMenuActions(actionButtons);

    if (gameState == GameState::MainMenu) {
        updateMainMenu(actionButtons);
    } else if (gameState == GameState::Pause) {
        updatePauseMenu(actionButtons);
    } else if (gameState == GameState::GameOver) {
        updateGameOverMenu(actionButtons);
    } else if (gameState == GameState::HelpMenu) {
        updateHelpMenu(actionButtons);
    } else if (gameState == GameState::ControllerHelp) {
        updateControllerHelpMenu(actionButtons);
    } else if (gameState == GameState::KeyboardHelp) {
        updateKeyboardHelpMenu(actionButtons);
    }
}

void RacingGame::handleMenuActions(MenuAction actionButtons)
{
    if (actionButtons == MenuAction::StartGame || actionButtons == MenuAction::ResumeGame) {
        if (actionButtons == MenuAction::StartGame) {
            gameTime.reset(glfwGetTime());
            racingSystem->restart();
        }
        audioManager->resumeChannel(inGameMusicChannelID);
        gameState = GameState::InGame;
    }
}

void RacingGame::updateMainMenu(MenuAction actionButtons)
{
    MenuAction actionCursor = menus->renderMainMenu();

    if (actionButtons == MenuAction::StartGame || actionCursor == MenuAction::StartGame) {
        racingSystem->restart();
        gameState = GameState::InGame;
    }
    else if (actionButtons == MenuAction::GoToHelpMenu || actionCursor == MenuAction::GoToHelpMenu) {
        gameState = GameState::HelpMenu;
    }

    updateMenuAudioState();
}

void RacingGame::updatePauseMenu(MenuAction actionButtons)
{
    MenuAction actionCursor = menus->renderPauseMenu();

    if (actionButtons == MenuAction::ResumeGame || actionCursor == MenuAction::ResumeGame) {
        gameState = GameState::InGame;
    }
    else if (actionButtons == MenuAction::GoToMainMenu || actionCursor == MenuAction::GoToMainMenu) {
        gameState = GameState::MainMenu;
    }

    updateMenuAudioState();
}

void RacingGame::updateGameOverMenu(MenuAction actionButtons)
{
    auto& playerRacer = gCoordinator.GetComponent<Racer>(playerVehicleEntity);
    int rank = playerRacer.currentRank;
    MenuAction actionCursor = menus->renderGameOver(rank, playerRacer.engulfed);

    if (actionButtons == MenuAction::GoToMainMenu || actionCursor == MenuAction::GoToMainMenu) {
        gameState = GameState::MainMenu;
    }

    updateMenuAudioState();
}

void RacingGame::updateHelpMenu(MenuAction actionButtons)
{
    MenuAction actionCursor = menus->renderHelpMenu();

    if (actionButtons == MenuAction::GoToMainMenu || actionCursor == MenuAction::GoToMainMenu) {
        gameState = GameState::MainMenu;
    }
    else if (actionButtons == MenuAction::GoToControllerHelp || actionCursor == MenuAction::GoToControllerHelp) {
        gameState = GameState::ControllerHelp;
    }
    else if (actionButtons == MenuAction::GoToKeyboardHelp || actionCursor == MenuAction::GoToKeyboardHelp) {
        gameState = GameState::KeyboardHelp;
    }

    updateMenuAudioState();
}

void RacingGame::updateControllerHelpMenu(MenuAction actionButtons)
{
    MenuAction actionCursor = menus->renderControllerHelp();

    if (actionButtons == MenuAction::GoToMainMenu || actionCursor == MenuAction::GoToMainMenu) {
        gameState = GameState::MainMenu;
    }

    updateMenuAudioState();
}

void RacingGame::updateKeyboardHelpMenu(MenuAction actionButtons)
{
    MenuAction actionCursor = menus->renderKeyboardHelp();

    if (actionButtons == MenuAction::GoToMainMenu || actionCursor == MenuAction::GoToMainMenu) {
        gameState = GameState::MainMenu;
    }

    updateMenuAudioState();
}

// ----- Gameplay helpers ----- //

void RacingGame::updatePhysicsAndGameplayLoop()
{
    // Physics System Loop, adaptive based on performance.
    size_t maxPhysicsSteps = gameTime.maxPhysicsSteps();
    size_t physicsSteps = 0;

    while (gameTime.accumulator >= gameTime.dt && physicsSteps < maxPhysicsSteps) {
        if (gameTime.frameCount < 300 && gameTime.physicsFrameCount > maxPhysicsSteps) {
            break; // Skip the first frames to avoid slow startup.
        }

        vehicleControlSystem->update(gameTime.dtF());
        aiSystem->update(gameTime.dtF());

        physicsSystem->update(gameTime.dtF());
        gameTime.physicsUpdate();
        physicsSteps++;

        racingSystem->update(gameTime.dtF());
        if (racingSystem->raceFinished) {
            gameState = GameState::GameOver;
        }
    }

    // Discard excess time when running slow to prevent spiral of death.
    if (physicsSteps >= maxPhysicsSteps) {
        gameTime.discardExcessTime();
    }
}

void RacingGame::updateInGameAudioState(float playerSpeed)
{
    // Avalanche distance-based volume
    glm::vec3 avalanchePos = gCoordinator.GetComponent<PhysxTransform>(AvalancheEntity).pos;
    glm::vec3 playerPos = gCoordinator.GetComponent<PhysxTransform>(playerVehicleEntity).pos;
    float distance = glm::length(avalanchePos - playerPos);

    float distanceRatio = std::clamp(distance / maxAudibleDistance, 0.0f, 1.0f);
    float volumeInDB = distanceRatio * -60.0f;
    float masterVolumeOffset = -5.0f;
    volumeInDB += masterVolumeOffset;
    audioManager->setChannelVolume(avalancheChannelID, volumeInDB);

    // Player engine sound speed gating
    if (playerSpeed > 1.0f) {
        if (!enginePlaying) {
            engineChannelID = audioManager->playSounds("assets/audio/snowmobiles-4-trimmed.mp3", { 0,0,0 }, -15.0f);
            enginePlaying = true;
        }
        else {
            // When returning from pause, the channel can be paused while still marked as playing.
            audioManager->resumeChannel(engineChannelID);
        }
    }
    else if (enginePlaying) {
        audioManager->pauseChannel(engineChannelID);
        enginePlaying = false;
    }

    // Resume AI engine channels
    audioManager->resumeChannel(aiEngineChannelID1);
    audioManager->resumeChannel(aiEngineChannelID2);

    // AI distance-based volume
    glm::vec3 aiRacer1Pos = gCoordinator.GetComponent<PhysxTransform>(aiVehicleEntity1).pos;
    glm::vec3 aiRacer2Pos = gCoordinator.GetComponent<PhysxTransform>(aiVehicleEntity2).pos;

    float distanceAi1 = glm::length(aiRacer1Pos - playerPos);
    float distanceAi2 = glm::length(aiRacer2Pos - playerPos);

    float distanceRatio1 = std::clamp(distanceAi1 / maxAudibleDistance, 0.0f, 1.0f);
    float distanceRatio2 = std::clamp(distanceAi2 / maxAudibleDistance, 0.0f, 1.0f);
    float volumeInDB1 = distanceRatio1 * -60.0f + masterVolumeOffset;
    float volumeInDB2 = distanceRatio2 * -60.0f + masterVolumeOffset;

    audioManager->setChannelVolume(aiEngineChannelID1, volumeInDB1);
    audioManager->setChannelVolume(aiEngineChannelID2, volumeInDB2);
}

void RacingGame::updateInGameCameraTarget(float playerSpeed)
{
    // If entity exists, update camera target to follow the player vehicle BEFORE rendering.
    // This prevents 1-frame lag that causes ghosting/phasing artifacts.
    if (!gCoordinator.HasComponent<PhysxTransform>(playerVehicleEntity)) {
        return;
    }

    auto& transform = gCoordinator.GetComponent<PhysxTransform>(playerVehicleEntity);
    auto& modelRenderable = gCoordinator.GetComponent<ModelRenderable>(playerVehicleEntity);

    // Target the visual center, not the physics origin.
    glm::vec3 visualOffset = transform.rot * modelRenderable.visualOffsetPos;
    glm::vec3 targetPos = transform.pos + visualOffset;
    targetPos.y += 2.f;

    glm::vec3 targetForward = transform.forward();
    renderingSystem->updateCameraTarget(targetPos, targetForward, playerSpeed);
}

void RacingGame::renderInGameHUD()
{
    textSystem->beginText();
    textSystem->loadFont("arial.ttf", 48);

    float marginX = 30.f;
    float screenHeight = 1440.f;
    float topY = screenHeight - 50.f;

    // --- Debug & System Info ---
    textSystem->renderText(
        "Rendered Frames: " + std::to_string(gameTime.frameCount),
        { marginX, topY - 20.f, 0.40f }, { 0.2f, 0.5f, 0.8f });

    textSystem->renderText(
        "Physics Frames: " + std::to_string(gameTime.physicsFrameCount),
        { marginX, topY - 40.f, 0.40f }, { 0.5f, 0.2f, 0.8f });

    textSystem->renderText(
        "Game FPS: " + std::to_string(static_cast<int>(1.0f / gameTime.fpsF())),
        { marginX, topY - 75.f, 0.75f }, { 0.9f, 0.9f, 0.4f });

    // --- Leaderboard Section ---
    float lbYStart = topY - 250.f;
    textSystem->renderText("LEADERBOARD :", { marginX + 2.0f, lbYStart - 2.0f, 0.85f }, { 1.f, 1.f, 1.f });
    textSystem->renderText("LEADERBOARD :", { marginX, lbYStart, 0.85f }, { 0.15f, 0.7f, 0.6f });

    for (size_t i = 0; i < racingSystem->leaderboard.size(); ++i) {
        Entity e = racingSystem->leaderboard[i];
        auto& racer = gCoordinator.GetComponent<Racer>(e);
        auto& vehicle = gCoordinator.GetComponent<VehicleComponent>(e);

        // Color: Gold for Player (ID 0), White/Grey for AI
        glm::vec3 color = racer.engulfed ? glm::vec3(0.8f, 0.25f, 0.15f) : (vehicle.playerID == 0) ? glm::vec3(1.0f, 0.8f, 0.0f) : glm::vec3(0.2f, 0.7f, 0.8f);
        std::string name = (vehicle.playerID == 0) ? "PLAYER" : "AI_" + std::to_string(e);
        std::string entry = std::to_string(i + 1) + ". " + name + (racer.engulfed ? " X engulfed" : "") + ", at " + std::to_string(static_cast<int>(racer.raceCompletion * 100)) + "%";

        float textX = marginX;
        float textY = lbYStart - 50.f - (i * 50.f);
        float scale = 0.75f;

        textSystem->renderText(entry, { textX + 2.0f, textY - 2.0f, scale }, { 0.0f, 0.0f, 0.0f });
        textSystem->renderText(entry, { textX, textY, scale }, color);
    }

    // -- Snowball throw --
    float centerX = screenHeight / 2.0f;
    float centerY = screenHeight / 2.0f;

    float snowballX = centerX * 1.0f;
    float snowballY = 100.f;
    float snowBallCoolDownPlayer = gCoordinator.GetComponent<VehicleComponent>(playerVehicleEntity).snowBallCooldown;
    textSystem->renderText("SNOWBALL" + (snowBallCoolDownPlayer > 0.f ? std::format(": {:.1f}", snowBallCoolDownPlayer) : " READY !"), { snowballX + 2.f, snowballY - 2.f, 1.0f }, { 1.0f, 1.0f, 1.0f });
    textSystem->renderText("SNOWBALL" + (snowBallCoolDownPlayer > 0.f ? std::format(": {:.1f}", snowBallCoolDownPlayer) : " READY !"), { snowballX, snowballY, 1.0f }, { 1.0f, 0.35f, 0.0f });

    // --- Crosshair / Center UI ---
    textSystem->renderText("+", { centerX - 5.f, centerY - 5.f, 0.75f }, { 1.f, 1.f, 1.f });

    // --- Input Controls Info (Top Right) ---
    float controlX = centerX * 1.4f;
    glm::vec3 controlsColor{ 0.35f, 0.55f, 0.10f };
    float contolsSize{ 0.55f };
    float controlsOffset{ 28.f };
    float offset{ 0.f };
    textSystem->renderText("CONTROLS", { controlX, topY, 0.75f }, { 0.55f, 0.8f, 0.15f });
    textSystem->renderText("Drive: RT-LT / W-S", { controlX, topY - (offset += controlsOffset), contolsSize }, controlsColor);
    textSystem->renderText("Steer: L-Stick / A-D", { controlX, topY - (offset += controlsOffset), contolsSize }, controlsColor);
    textSystem->renderText("Boost: Y-Btn / SHIFT", { controlX, topY - (offset += controlsOffset), contolsSize }, controlsColor);
    textSystem->renderText("Shoot: X-Btn / SPACE", { controlX, topY - (offset += controlsOffset), contolsSize }, controlsColor);
    textSystem->renderText("Pause: Start / P", { controlX, topY - (offset += controlsOffset), contolsSize }, controlsColor);

    textSystem->endText();
}

// ----- UI and utility helpers ----- //

void RacingGame::updateMenuAudioState()
{
    // if NOT in game, don't play in-game music
    audioManager->pauseChannel(inGameMusicChannelID);
    // if on menu, play lobby music
    audioManager->resumeChannel(musicChannelID);
    // pause avalanche sounds in menus
    audioManager->pauseChannel(avalancheChannelID);
    // no engine sounds in menus
    audioManager->pauseChannel(engineChannelID);
    audioManager->pauseChannel(aiEngineChannelID1);
    audioManager->pauseChannel(aiEngineChannelID2);
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

    // Update camera info for UI
    imguiPanel->cameraInfo = renderingSystem->getActiveCameraInfo();
    imguiPanel->aspectRatio = window->getAspectRatio();

    // Update UI
    imguiWrapper->beginFrame();
    imguiPanel->update();
    imguiWrapper->renderFPS();
    syncImgui();
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
