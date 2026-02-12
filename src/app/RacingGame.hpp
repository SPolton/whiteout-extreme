#pragma once

#include "app/GameTime.hpp"
#include "core/RenderingSystem.hpp"
#include "core/Text.h"
#include "input/VehicleControlSystem.hpp"
#include "physics/PhysicsSystem.hpp"

#include <iostream>
#include <memory>

class RacingGame {
public:
    RacingGame();

    void run();

private:
    GameTime gameTime;
    std::shared_ptr<RenderingSystem> renderingSystem;
    std::shared_ptr<PhysicsSystem> physicsSystem;
    std::shared_ptr<VehicleControlSystem> vehicleControlSystem;

    std::unique_ptr<Text> textSystem;

    Entity playerVehicleEntity;
    Entity Earth;
    Entity Mars;
    Entity WoodyModel;
    Entity BackpackModel;

    // Pause Menu
    void togglePause();
    void renderPauseMenu();

    // Main Menu
    void toggleMainMenu();
    void renderMainMenu();

    // inputs for menus
    std::shared_ptr<InputManager> inputManager;

    // game state (default start on main menu)
    int gameState = 0;
    /*
    * 0 = main menu
    * 1 = in game
    * 2 = paused
    * 3 = game over (not used yet)
    */
};
