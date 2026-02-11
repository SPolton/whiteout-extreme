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

    // Pause Menu
    bool paused = false;
    void togglePause();
    void renderPauseMenu();

    // Main Menu
    bool mainMenu = true; // start on main menu first
    void toggleMainMenu();
    void renderMainMenu();

    // inputs for menus
    InputManager* inputManager;
};
