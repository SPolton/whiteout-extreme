#include "GameMenus.hpp"
#include "app/GameState.hpp"
#include <gtc/type_ptr.hpp>

GameMenus::GameMenus(Text* textSystem, InputManager* inputManager, AudioEngine* audioManager, Window* window, GameState& gameState)
    : textSystem(textSystem), inputManager(inputManager), audioManager(audioManager), window(window), gameState(gameState)
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

void GameMenus::init()
{
    // load the basic shader that only requires the following info: index positions and uvs
    try
    {
        shader = assetManager.loadShader("basic");
        logger::info("Basic shader loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load basic shader: {0}", e.what());
    }

    // pre-load the texture for the logo
    try
    {
        logoTexture = assetManager.loadTexture("assets/ui/WhiteoutExtreme.png");
        logger::info("Logo texture loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load logo texture: {0}", e.what());
    }

    // set the position of the quad of the logo
    quad.positions = {
        // Triangle 1
        {-1.f, -1.f, 0.f},
        { 1.f, -1.f, 0.f},
        { 1.f,  1.f, 0.f},
        // Triangle 2
        {-1.f, -1.f, 0.f},
        { 1.f,  1.f, 0.f},
        {-1.f,  1.f, 0.f},
    };
    quad.uvs = {
        // Triangle 1
        {0.f, 0.f},
        {1.f, 0.f},
        {1.f, 1.f},
        // Triangle 2
        {0.f, 0.f},
        {1.f, 1.f},
        {0.f, 1.f},
    };
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
            // if on main menu, then start game
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
            else if (gameState == GameState::GameOver) {
                // play UI button clicked sound
                audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
                return MenuAction::GoToMainMenu;
            }
            else if (gameState == GameState::HelpMenu) {
                // play UI button clicked sound
                audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
                return MenuAction::None;
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
        // triggers help menu (can only be navigated to from main menu)
        // TODO: figure out what button on controller to trigger help menu
        else if (inputManager->isControllerButtonPressedOnce(GLFW_GAMEPAD_BUTTON_Y) && gameState == GameState::MainMenu) {
            audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
            gameState = GameState::HelpMenu; // update game state to render help page
            return MenuAction::None; // no action taken
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
        // if on main menu, then start game
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
        else if (gameState == GameState::GameOver) {
            // play UI button clicked sound
            audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
            return MenuAction::GoToMainMenu;
        }
        else if (gameState == GameState::HelpMenu) {
            // play UI button clicked sound
            audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
            return MenuAction::GoToMainMenu;
        }
        else if (gameState == GameState::ControllerHelp) {
            // play UI button clicked sound
            audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
            return MenuAction::GoToMainMenu;
        }
        else if (gameState == GameState::KeyboardHelp) {
            // play UI button clicked sound
            audioManager->playSounds("assets/audio/menu-button.mp3", { 0,0,0 }, -8.0f);
            return MenuAction::GoToMainMenu;
        }
    }
    // triggers help menu (can only be navigated to from main menu)
    else if (inputManager->isKeyPressedOnce(GLFW_KEY_H) && gameState == GameState::MainMenu) {
        audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
        gameState = GameState::HelpMenu; // update game state to render help page
        return MenuAction::None; // no action taken
    }
    // triggers controller help menu (can only be navigated to from Help menu)
    else if (inputManager->isKeyPressedOnce(GLFW_KEY_J) && gameState == GameState::HelpMenu) {
        audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
        gameState = GameState::ControllerHelp; // update game state to render help page
        return MenuAction::None; // no action taken
    }
    // triggers keyboard help menu (can only be navigated to from Help menu)
    else if (inputManager->isKeyPressedOnce(GLFW_KEY_K) && gameState == GameState::HelpMenu) {
        audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
        gameState = GameState::KeyboardHelp; // update game state to render help page
        return MenuAction::None; // no action taken
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

    // create vao to draw menu logo
    GPU_Geometry gpuQuad;
    gpuQuad.Update2D(quad); // update it with our basic quad info

    shader->use();

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    logoTexture->bind();
    glUniform1i(glGetUniformLocation(*shader, "sample"), 0);

    // translations
    float translateY = 0.25f;

    // static model to pass to shader, renders png as is
    glm::mat4 model = glm::mat4(
        1.f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.f, 0.0f, 0.0f,
        0.f, 0.0f, 1.f, 0.0f,
        0.0f, translateY, 0.f, 1.0f
    );

    glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // bind vao
    gpuQuad.bind();

    // draw quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

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
    textSystem->renderText("Help", { 605.f, 250.f, 0.75f }, defaultColor);

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


MenuAction GameMenus::renderGameOver(int rank, bool engulfed)
{
    // Clear buffers
    glClearColor(0.6f, 0.8f, 1.0f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // when in a menu, check for cursor position to highlight "buttons"
    // get cursor position
    glm::dvec2 cursorPos = inputManager->cursorPosition();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    // 1. Determine the correct English ordinal suffix
    std::string suffix = "th";
    if (rank % 10 == 1 && rank % 100 != 11) suffix = "st !";
    else if (rank % 10 == 2 && rank % 100 != 12) suffix = "nd";
    else if (rank % 10 == 3 && rank % 100 != 13) suffix = "rd";

    // 2. Prepare the result string
    std::string rankText = std::to_string(rank) + suffix; // +" Place";

    // 3. Render Final UI
    textSystem->renderText("Game Over!", { 470.f, 1100.f, 0.75f }, { 0.f, 0.f, 0.55f });
    textSystem->renderText("You finished", { 470.f, 900.f, 0.65f }, { 0.f, 0.f, 0.55f });

    // Display the Rank (positioned 100 pixels below the previous line)
    textSystem->renderText(rankText, { 600.f, 700.f, 1.0f }, { 0.8f, 0.2f, 1.0f });

    if(engulfed) textSystem->renderText("ENGULFED !", {525.f, 600.f, 0.65f}, { 0.9f, 0.35f, 0.20f });

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

MenuAction GameMenus::renderHelpMenu()
{
    // Clear buffers
    glClearColor(0.6f, 0.8f, 1.0f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // when in a menu, check for cursor position to highlight "buttons"
    // get cursor position
    glm::dvec2 cursorPos = inputManager->cursorPosition();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    // default color for the "Back" button
    glm::vec3 defaultColor = { 0.f, 0.f, 0.6f };

    // set the "Back" button to the default color (used when not hovered upon)
    glm::vec3 backColor = defaultColor;

    // check if mouse is hovered over the "Back" button
    if (cursorPos.x > 485.f && cursorPos.x < 700.f) {
        if (cursorPos.y > 540.f && cursorPos.y < 580.f) {
            // if it is, highlight in red
            backColor = { 0.8f, 0.f, 0.f };

            // and check if the user clicks on the mouse while over the "Back" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play an "entering game" sound when button clicked
                audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
                // if they do, toggle to show the main menu
                return MenuAction::GoToMainMenu;
            }
        }
    }

    // pages to choose from
    textSystem->renderText("Keyboard", { 500.f, 900.f, 0.75f }, defaultColor);
    textSystem->renderText("Controller", { 460.f, 700.f, 0.75f }, defaultColor);

    // render the text with the proper color assigned
    textSystem->renderText("Back", { 585.f, 400.f, 0.75f }, backColor);

    textSystem->endText();

    // default return
    return MenuAction::None;
}

MenuAction GameMenus::renderControllerHelp()
{
    // Clear buffers
    glClearColor(0.6f, 0.8f, 1.0f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // when in a menu, check for cursor position to highlight "buttons"
    // get cursor position
    glm::dvec2 cursorPos = inputManager->cursorPosition();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    // default color for the "Start" button
    glm::vec3 defaultColor = { 0.f, 0.f, 0.6f };

    // set the "return" button to the default color (used when not hovered upon)
    glm::vec3 startColor = defaultColor;

    // check if mouse is hovered over the "return" button
    if (cursorPos.x > 485.f && cursorPos.x < 700.f) {
        if (cursorPos.y > 540.f && cursorPos.y < 580.f) {
            // if it is, highlight in red
            startColor = { 0.8f, 0.f, 0.f };

            // and check if the user clicks on the mouse while over the "return" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play ui button sound when button clicked
                audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
                // if they do, toggle to show the main menu
                return MenuAction::GoToMainMenu;
            }
        }
    }

    // render the text with the proper color assigned
    textSystem->renderText("Controller", { 400.f, 1300.f, 0.75f }, defaultColor);

    // create vao to draw menu logo
    GPU_Geometry gpuQuad;
    gpuQuad.Update2D(quad); // update it with our basic quad info

    shader->use();

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    logoTexture->bind();
    glUniform1i(glGetUniformLocation(*shader, "sample"), 0);

    // translations
    float translateY = 0.25f;

    // static model to pass to shader, renders png as is
    glm::mat4 model = glm::mat4(
        1.f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.f, 0.0f, 0.0f,
        0.f, 0.0f, 1.f, 0.0f,
        0.0f, translateY, 0.f, 1.0f
    );

    glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // bind vao
    gpuQuad.bind();

    // draw quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // return to main menu button
    textSystem->renderText("Return to Main Menu", { 300.f, 400.f, 0.75f }, defaultColor);

    textSystem->endText();

    // default return
    return MenuAction::None;
}

MenuAction GameMenus::renderKeyboardHelp()
{
    // Clear buffers
    glClearColor(0.6f, 0.8f, 1.0f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // when in a menu, check for cursor position to highlight "buttons"
    // get cursor position
    glm::dvec2 cursorPos = inputManager->cursorPosition();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    // default color for the "Start" button
    glm::vec3 defaultColor = { 0.f, 0.f, 0.6f };

    // set the "return" button to the default color (used when not hovered upon)
    glm::vec3 startColor = defaultColor;

    // check if mouse is hovered over the "return" button
    if (cursorPos.x > 485.f && cursorPos.x < 700.f) {
        if (cursorPos.y > 540.f && cursorPos.y < 580.f) {
            // if it is, highlight in red
            startColor = { 0.8f, 0.f, 0.f };

            // and check if the user clicks on the mouse while over the "return" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play ui button sound when button clicked
                audioManager->playSounds("assets/audio/game-start.mp3", { 0,0,0 }, -8.0f);
                // if they do, toggle to show the main menu
                return MenuAction::GoToMainMenu;
            }
        }
    }

    // render the text with the proper color assigned
    textSystem->renderText("Keyboard", { 500.f, 1300.f, 0.75f }, defaultColor);

    // create vao to draw menu logo
    GPU_Geometry gpuQuad;
    gpuQuad.Update2D(quad); // update it with our basic quad info

    shader->use();

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    logoTexture->bind();
    glUniform1i(glGetUniformLocation(*shader, "sample"), 0);

    // translations
    float translateY = 0.25f;

    // static model to pass to shader, renders png as is
    glm::mat4 model = glm::mat4(
        1.f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.f, 0.0f, 0.0f,
        0.f, 0.0f, 1.f, 0.0f,
        0.0f, translateY, 0.f, 1.0f
    );

    glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // bind vao
    gpuQuad.bind();

    // draw quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // return to main menu button
    textSystem->renderText("Return to Main Menu", { 300.f, 400.f, 0.75f }, defaultColor);

    textSystem->endText();

    // default return
    return MenuAction::None;
}

