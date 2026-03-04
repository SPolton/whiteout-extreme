#pragma once

#include "app/GameState.hpp"
#include "core/Text.h"
#include "input/glfw/InputManager.hpp"

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
    MenuAction renderGameOver(int rank);

    // input related functions
    MenuAction pollInputs();
    void checkInputSystem();

private:
    // pointers to read input
    Text* textSystem;
    InputManager* inputManager;

    // get game state
    GameState& gameState;

    /*
    * keep track of which input system to use
    * by default 0 = keyboard/mouse
    * 1 = controller
    */
    int inputSystem = 0;
};
