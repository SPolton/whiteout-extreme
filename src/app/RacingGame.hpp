#pragma once

#include "app/GameTime.hpp"
#include "core/RenderingSystem.hpp"
#include "core/Text.h"
#include "input/VehicleControlSystem.hpp"
#include "physics/PhysicsSystem.hpp"
#include "ui/GameMenus.hpp"
#include "GameState.hpp"

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

    // access UI menus
    std::unique_ptr<GameMenus> menus;

    // set default game state on main menu
    GameState gameState = GameState::MainMenu;
};
