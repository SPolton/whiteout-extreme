#pragma once

#include "core/Text.h"
#include "input/InputManager.hpp"
#include "app/GameState.hpp"

// types of actions to take
enum class MenuAction {
    None,
    StartGame,
    ResumeGame,
    GoToMainMenu
};

class GameMenus {
public:
    GameMenus(Text* textSystem, InputManager* inputManager, GameState& gameState);

    // returns the action taken
    MenuAction renderMainMenu();
    MenuAction renderPauseMenu();

    MenuAction pollInputs();

private:
    // pointers to read input
    Text* textSystem;
    InputManager* inputManager;

    // get game state
    GameState& gameState;
};
