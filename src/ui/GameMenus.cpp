#include "GameMenus.hpp"
#include "app/GameState.hpp"

GameMenus::GameMenus(Text* textSystem, InputManager* inputManager, AudioEngine* audioManager, GameState& gameState)
    : textSystem(textSystem), inputManager(inputManager), audioManager(audioManager), gameState(gameState)
{
}

void GameMenus::checkInputSystem() {
    // poll controller state first
    inputManager->pollControllerInputs();

    // based on whether controller is connected
    if (inputManager->isControllerConnected()) {
        inputSystem = 1; // set input system to controller
    }
    else {
        // otherwise default keyboard/mouse input
        inputSystem = 0;
    }
}

void GameMenus::loadMenuSounds()
{
    // load sound to play when entering game
    audioManager->loadSound("assets/audio/game-start.mp3", false, false, false);
    // load sound for pressing menu buttons
    audioManager->loadSound("assets/audio/menu-button.mp3", false, false, false);
}

MenuAction GameMenus::pollInputs() {
    // check for controller inputs
    if (inputSystem == 1) {
        // triggers pause menu
        if (inputManager->isControllerButtonPressedOnce(GLFW_GAMEPAD_BUTTON_START)) {
            // if in game, render pause menu
            if (gameState == GameState::InGame) {
                gameState = GameState::Pause;
                // play UI button clicked sound
                audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
                return MenuAction::None;
            }
            else if (gameState == GameState::Pause) {
                // play an "entering game" sound when button clicked
                audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
                return MenuAction::ResumeGame;
            }
        }
        // triggers main menu (do not allow keyboard input to navigate to main menu while in game)
        else if (inputManager->isControllerButtonPressedOnce(GLFW_GAMEPAD_BUTTON_A) && gameState != GameState::InGame) {
            // if one main menu, then start game
            if (gameState == GameState::MainMenu) {
                // play an "entering game" sound when button clicked
                audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
                return MenuAction::StartGame;
            }
            else if (gameState == GameState::Pause) {
                // play an "entering game" sound when button clicked
                audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
                return MenuAction::ResumeGame;
            }
        }
        // B button to go back from pause to main menu
        else if (inputManager->isControllerButtonPressedOnce(GLFW_GAMEPAD_BUTTON_B)) {
            // if in pause menu, render main menu
            if (gameState == GameState::Pause) {
                gameState = GameState::MainMenu; // update game state
                // play UI button clicked sound
                audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
                return MenuAction::GoToMainMenu;
            }
        }
    }

    // otherwise keyboard input works too
    // triggers pause menu
    if (inputManager->isKeyPressedOnce(GLFW_KEY_P)) {
        // if paused, then resume game
        if (gameState == GameState::Pause) {
            // play an "entering game" sound when button clicked
            audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
            return MenuAction::ResumeGame;
        }
        // if in game, render pause menu
        else if (gameState == GameState::InGame) {
            gameState = GameState::Pause;
            // play UI button clicked sound
            audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
            return MenuAction::None;
        }
    }
    // triggers main menu (do not allow keyboard input to navigate to main menu while in game)
    else if (inputManager->isKeyPressedOnce(GLFW_KEY_M) && gameState != GameState::InGame) {
        // if one main menu, then start game
        if (gameState == GameState::MainMenu) {
            // play an "entering game" sound when button clicked
            audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
            return MenuAction::StartGame;
        }
        else if (gameState == GameState::Pause) {
            // play UI button clicked sound
            audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
            return MenuAction::GoToMainMenu;
        }
    }

    return MenuAction::None;
}

MenuAction GameMenus::renderMainMenu()
{
    // Clear buffers
    glClearColor(0.6f, 0.8f, 1.0f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // when in a menu, check for cursor position to highlight "buttons"
    // get cursor position
    glm::dvec2 cursorPos = inputManager->cursorPosition();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    textSystem->renderText("Whiteout Extreme", { 300.f, 1100.f, 0.75f }, { 0.f, 0.f, 0.55f });

    // default color for the "Start" button
    glm::vec3 defaultColor = { 0.f, 0.f, 0.6f };

    // set the "Start" button to the default color (used when not hovered upon)
    glm::vec3 startColor = defaultColor;

    // check if mouse is hovered over the "Start" button
    if (cursorPos.x > 485.f && cursorPos.x < 700.f) {
        if (cursorPos.y > 540.f && cursorPos.y < 580.f) {
            // if it is, highlight in red
            startColor = { 0.8f, 0.f, 0.f };

            // and check if the user clicks on the mouse while over the "Start" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play an "entering game" sound when button clicked
                audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
                // if they do, toggle to NOT show the main menu
                return MenuAction::StartGame;
            }
        }
    }

    // render the text with the proper color assigned
    textSystem->renderText("Start", { 585.f, 400.f, 0.75f }, startColor);

    textSystem->endText();

    // default return
    return MenuAction::None;
}

MenuAction GameMenus::renderPauseMenu() {
    // Clear buffers
    glClearColor(0.6f, 0.8f, 1.0f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // when in a menu, check for cursor position to highlight "buttons"
    // get cursor position
    glm::dvec2 cursorPos = inputManager->cursorPosition();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    textSystem->renderText("PAUSE MENU", { 460.f, 1100.f, 0.75f }, { 0.f, 0.f, 0.55f });

    textSystem->loadFont("SNPro-SemiBold.ttf", 85);

    // default color for the buttons
    glm::vec3 defaultColor = { 0.f, 0.f, 0.6f };

    // set the buttons to the default color (used when not hovered upon)
    glm::vec3 resumeColor = defaultColor;
    glm::vec3 quitColor = defaultColor;

    // check if mouse is hovered over the "Resume" button
    if (cursorPos.x > 495.f && cursorPos.x < 685.f) {
        if (cursorPos.y > 270.f && cursorPos.y < 300.f) {
            // if it is, highlight in red
            resumeColor = { 0.8f, 0.f, 0.f };

            // and check if the user clicks on the mouse while over the "Resume" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play an "entering game" sound when button clicked
                audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
                // if they do, resume the game
                return MenuAction::ResumeGame;
            }
        }
    }

    // check if mouse is hovered over the "Quit" button
    if (cursorPos.x > 310.f && cursorPos.x < 895.f) {
        if (cursorPos.y > 355.f && cursorPos.y < 385.f) {
            // if it is, highlight in red
            quitColor = { 0.8f, 0.f, 0.f };

            // and check if the user clicks on the mouse while over the "Quit" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play UI button clicked sound
                audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
                // and toggle to show the main menu
                return MenuAction::GoToMainMenu;
            }
        }
    }

    // render the text with the proper color assigned
    textSystem->renderText("Resume", { 590.f, 900.f, 0.75f }, resumeColor);
    textSystem->renderText("Quit (Exit to Main Menu)", { 370.f, 750.f, 0.75f }, quitColor);

    textSystem->endText();

    // default return
    return MenuAction::None;
}


MenuAction GameMenus::renderGameOver()
{
    // Clear buffers
    glClearColor(0.6f, 0.8f, 1.0f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // when in a menu, check for cursor position to highlight "buttons"
    // get cursor position
    glm::dvec2 cursorPos = inputManager->cursorPosition();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    textSystem->renderText("Game Over!", { 470.f, 1100.f, 0.75f }, { 0.f, 0.f, 0.55f });

    // default color for the "Return to menu" button
    glm::vec3 defaultColor = { 0.f, 0.f, 0.6f };

    // set the "Return to menu" button to the default color (used when not hovered upon)
    glm::vec3 startColor = defaultColor;

    // check for controller inputs
    if (inputSystem == 1) {
        // acknowledge game is over by clicking confirm (A)
        if (inputManager->isControllerButtonPressedOnce(GLFW_GAMEPAD_BUTTON_A)) {
            // play UI button clicked sound
            audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
            // if "A" is pressed (bottom button?), go to main menu
            return MenuAction::GoToMainMenu;
        }
    }
    // otherwise it must be keyboard/mouse input
    // check if mouse is hovered over the "Return to menu" button
    if (cursorPos.x > 225.f && cursorPos.x < 1000.f) {
        if (cursorPos.y > 540.f && cursorPos.y < 575.f) {
            // if it is, highlight in red
            startColor = { 0.8f, 0.f, 0.f };

            // and check if the user clicks on the mouse while over the "Return to menu" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play UI button clicked sound
                audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
                // if they do, toggle to show the main menu
                return MenuAction::GoToMainMenu;
            }
        }
    }

    // render the text with the proper color assigned
    textSystem->renderText("Return to Main Menu", { 270.f, 400.f, 0.75f }, startColor);

    textSystem->endText();

    // default return
    return MenuAction::None;
}
