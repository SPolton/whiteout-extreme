#pragma once

#include "app/GameState.hpp"
#include "core/Text.h"
#include "input/glfw/InputManager.hpp"
#include "audio/AudioEngine.h"

#include "core/assets/AssetManager.hpp"
#include "core/assets/Texture.hpp"
#include "core/buffer/Geometry.hpp"
#include "core/render/ShaderProgram.hpp"
#include "core/scene/Transform.hpp"

// types of actions to take
enum class MenuAction {
    None,
    StartGame,
    ResumeGame,
    GoToMainMenu
};

class GameMenus {
public:
    GameMenus(Text* textSystem, InputManager* inputManager, AudioEngine* audioManager, Window* window, GameState& gameState);

    // returns the action taken
    MenuAction renderMainMenu();
    MenuAction renderPauseMenu();
    MenuAction renderGameOver(int rank, bool engulfed);
    MenuAction renderHelpMenu();
    MenuAction renderControllerHelp();
    MenuAction renderKeyboardHelp();


    // input related functions
    MenuAction pollInputs();
    void checkInputSystem();

    // load sounds to use on menus
    void loadMenuSounds();

    // load textures and shader program
    void init();

private:
    // Textures
    AssetManager& assetManager = AssetManager::getInstance();
    std::shared_ptr<Texture> logoTexture;
    std::shared_ptr<Texture> backgroundTexture;
    std::shared_ptr<Texture> keyboardTexture;
    std::shared_ptr<Texture> controllerTexture;
    std::shared_ptr<ShaderProgram> shader;

    // pointers to read input
    Text* textSystem;
    InputManager* inputManager;
    // pointer to call audio engine
    AudioEngine* audioManager;
    // pointer to window
    Window* window;

    // get game state
    GameState& gameState;

    /*
    * keep track of which input system to use
    * by default 0 = keyboard/mouse
    * 1 = controller
    */
    int inputSystem = 0;

    // get some window properties to use
    // default we use 1080 x 720
    float defaultWindowWidth = 1080.f;
    float defaultWindowHeight = 720.f;

    // used to display image textures
    CPU_Geometry quad;
};
