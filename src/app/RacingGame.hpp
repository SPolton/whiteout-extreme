#pragma once

#include "app/GameTime.hpp"
#include "core/RenderingSystem.hpp"
#include "core/Text.h"
#include "ai/RacingSystem.hpp"
#include "ai/AISystem.hpp"
#include "input/VehicleControlSystem.hpp"
#include "physics/PhysicsSystem.hpp"
#include "audio/AudioEngine.h"
#include "ui/GameMenus.hpp"
#include "GameState.hpp"
#include "input/glfw/Window.hpp"
#include "input/glfw/InputManager.hpp"
#include "input/panel/ImGuiWrapper.hpp"
#include "input/panel/ImGuiPanel.hpp"

#include <iostream>
#include <memory>
#include <format>

class RacingGame {
public:
    RacingGame();
    ~RacingGame();

    void run();

private:
    // State handlers
    void updateInGame();
    void updateInMenu();
    void handleMenuActions(MenuAction actionButtons);
    void updateMainMenu(MenuAction actionButtons);
    void updatePauseMenu(MenuAction actionButtons);
    void updateGameOverMenu(MenuAction actionButtons);

    // Gameplay
    void updatePhysicsAndGameplayLoop();
    void updateInGameAudioState(float playerSpeed);
    void updateInGameCameraTarget(float playerSpeed);
    void renderInGameHUD();

    // UI and utility
    void updateMenuAudioState();
    void updateImGui();
    void syncImgui();
    void endFrame();

    GameTime gameTime;

    std::shared_ptr<InputManager> inputManager;
    std::shared_ptr<Window> window;

    std::shared_ptr<ImGuiWrapper> imguiWrapper;
    std::shared_ptr<ImGuiPanel> imguiPanel;

    std::shared_ptr<RenderingSystem> renderingSystem;
    std::shared_ptr<PhysicsSystem> physicsSystem;
    std::shared_ptr<VehicleControlSystem> vehicleControlSystem;
    std::shared_ptr<RacingSystem> racingSystem;
    std::shared_ptr<AISystem> aiSystem;

    std::unique_ptr<Text> textSystem;

    // access UI menus
    std::unique_ptr<GameMenus> menus;
    // set default game state on main menu
    GameState gameState = GameState::MainMenu;

    //Entity playerVehicleEntity;
    Entity GroundPlane = 0;
    Entity MapModel = 0;
    Entity Skybox = 0;
    Entity Waypoint = 0;
    Entity AvalancheEntity = 0;

    /*
    Entity Earth = 0;
    Entity Mars = 0;
    Entity WoodyModel = 0;
    Entity BackpackModel = 0;
    */

    // sound
    std::shared_ptr<AudioEngine> audioManager;
    int musicChannelID = -1;
    int inGameMusicChannelID = -1;
    int avalancheChannelID = -1;
    // keep track of engine sound playing state
    int engineChannelID = -1;
    bool enginePlaying = false;
    // avalanche range
    float maxAudibleDistance = 300.f;
    // ai engine
    int aiEngineChannelID1 = -1;
    int aiEngineChannelID2 = -1;
};
