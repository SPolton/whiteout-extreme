#pragma once

#include "app/GameTime.hpp"
#include "core/RenderingSystem.hpp"
#include "core/Text.h"
#include "ai/RacingSystem.hpp"
#include "ai/AISystem.hpp"
#include "input/VehicleControlSystem.hpp"
#include "physics/PhysicsSystem.hpp"
#include "ui/GameMenus.hpp"
#include "GameState.hpp"
#include "input/glfw/Window.hpp"
#include "input/glfw/InputManager.hpp"
#include "input/panel/ImGuiWrapper.hpp"
#include "input/panel/ImGuiPanel.hpp"

#include <iostream>
#include <memory>

class RacingGame {
public:
    RacingGame();

    void run();

private:
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
    Entity MapModel = 0;
    Entity Skybox = 0;
    Entity Waypoint = 0;

    /*
    Entity Earth = 0;
    Entity Mars = 0;
    Entity WoodyModel = 0;
    Entity BackpackModel = 0;
    */

};
