#pragma once

#include "app/GameTime.hpp"
#include "core/RenderingSystem.hpp"
#include "core/Text.h"
#include "input/VehicleControlSystem.hpp"
#include "physics/PhysicsSystem.hpp"
#include "audio/AudioSystem.hpp"
#include "ui/GameMenus.hpp"
#include "GameState.hpp"

#include <iostream>
#include <memory>

class RacingGame {
public:
    RacingGame();

    void run();
    void music();

private:
    GameTime gameTime;
    std::shared_ptr<VehicleControlSystem> vehicleControlSystem;

    std::unique_ptr<Text> textSystem;

    // access UI menus
    std::unique_ptr<GameMenus> menus;
    // set default game state on main menu
    GameState gameState = GameState::MainMenu;

    //Entity playerVehicleEntity;
    Entity Earth;
    Entity Mars;
    Entity WoodyModel;
    Entity BackpackModel;
    Entity MapModel;

    // test sound
    FMOD::Studio::System* audioStudio = nullptr;
    FMOD::System* coreSystem = nullptr;
    FMOD::Sound* musicSound = nullptr;
    FMOD::Channel* musicChannel = nullptr;
};
