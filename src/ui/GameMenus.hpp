#pragma once

#include "app/GameState.hpp"
#include "core/Text.h"
#include "input/glfw/InputManager.hpp"
#include "audio/AudioEngine.h"

// types of actions to take
enum class MenuAction {
    None,
    StartGame,
    ResumeGame,
    GoToMainMenu
};

class GameMenus {
public:
    GameMenus(Text* textSystem, InputManager* inputManager, AudioEngine* audioManager, GameState& gameState);

    // returns the action taken
    MenuAction renderMainMenu();
    MenuAction renderPauseMenu();
    MenuAction renderGameOver();

    // input related functions
    MenuAction pollInputs();
    void checkInputSystem();

    // load sounds to use on menus
    void loadMenuSounds();

private:
    // pointers to read input
    Text* textSystem;
    InputManager* inputManager;
    // pointer to call audio engine
    AudioEngine* audioManager;

    // get game state
    GameState& gameState;

    /*
    * keep track of which input system to use
    * by default 0 = keyboard/mouse
    * 1 = controller
    */
    int inputSystem = 0;
};
