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
    // Ensure audio registry is loaded before menu actions trigger sounds.
    audioManager->loadSoundRegistry();
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

    // pre-load the texture for the main menu background picture
    try
    {
        backgroundTexture = assetManager.loadTexture("assets/ui/ChatGPT_Image.png");
        logger::info("Background picture texture loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load background picture texture: {0}", e.what());
    }

    // pre-load the texture for the help menu keyboard controls
    try
    {
        keyboardTexture = assetManager.loadTexture("assets/ui/Keyboard_Controls_1.jpeg");
        logger::info("Keyboard controls texture loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load keyboard controls texture: {0}", e.what());
    }

    // pre-load the texture for the help menu controller controls
    try
    {
        controllerTexture = assetManager.loadTexture("assets/ui/Controller_Controls_1.jpeg");
        logger::info("Controller controls texture loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load controller controls texture: {0}", e.what());
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
    // poll controller state first
    inputManager->pollControllerInputs();

    // check for controller inputs
    if (inputSystem == 1) {
        // triggers pause menu
        if (inputManager->isControllerButtonPressedOnce(GLFW_GAMEPAD_BUTTON_START)) {
            // if in game, render pause menu
            if (gameState == GameState::InGame) {
                gameState = GameState::Pause;
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
                return MenuAction::None;
            }
            else if (gameState == GameState::Pause) {
                // play an "entering game" sound when button clicked
                audioManager->jsonSound("ui.game.start");
                return MenuAction::ResumeGame;
            }
        }
        // triggers main menu (do not allow keyboard input to navigate to main menu while in game)
        else if (inputManager->isControllerButtonPressedOnce(GLFW_GAMEPAD_BUTTON_A) && gameState != GameState::InGame) {
            // MAIN MENU
            // if on main menu and player selected "start", then start game
            if (gameState == GameState::MainMenu && selectedMenuOption == 0) {
                // play an "entering game" sound when button clicked
                audioManager->jsonSound("ui.game.start");
                return MenuAction::StartGame;
            }
            // if on main menu and player selected "help", then go to help menu
            else if (gameState == GameState::MainMenu && selectedMenuOption == 1) {
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
                return MenuAction::GoToHelpMenu;
            }
            // PAUSE MENU
            else if (gameState == GameState::Pause && selectedPauseMenuOption == 0) {
                // play an "entering game" sound when button clicked
                audioManager->jsonSound("ui.game.start");
                return MenuAction::ResumeGame;
            }
            else if (gameState == GameState::Pause && selectedPauseMenuOption == 1) {
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
                return MenuAction::GoToMainMenu;
            }
            else if (gameState == GameState::Pause) {
                // play an "entering game" sound when button clicked
                audioManager->jsonSound("ui.game.start");
                return MenuAction::ResumeGame;
            }
            // GAME OVER SCREEN
            else if (gameState == GameState::GameOver) {
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
                return MenuAction::GoToMainMenu;
            }
            // HELP MENU
            else if (gameState == GameState::HelpMenu && selectedHelpMenuOption == 0) {
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
                return MenuAction::GoToKeyboardHelp;
            }
            else if (gameState == GameState::HelpMenu && selectedHelpMenuOption == 1) {
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
                return MenuAction::GoToControllerHelp;
            }
            else if (gameState == GameState::HelpMenu && selectedHelpMenuOption == 2) {
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
                return MenuAction::GoToMainMenu;
            }
            // HELP MENU SUBPAGES
            // only one option to return to main menu, so it is auto selected
            else if (gameState == GameState::ControllerHelp || gameState == GameState::KeyboardHelp) {
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
                return MenuAction::GoToMainMenu;
            }
        }
        // B button to go back from pause to main menu
        else if (inputManager->isControllerButtonPressedOnce(GLFW_GAMEPAD_BUTTON_B)) {
            // if in pause menu, render main menu
            if (gameState == GameState::Pause) {
                gameState = GameState::MainMenu; // update game state
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
                return MenuAction::GoToMainMenu;
            }
            // HELP MENU
            else if (gameState == GameState::HelpMenu) {
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
                return MenuAction::GoToMainMenu;
            }
            else if (gameState == GameState::KeyboardHelp) {
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
                return MenuAction::GoToMainMenu;
            }
            else if (gameState == GameState::ControllerHelp) {
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
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
            audioManager->jsonSound("ui.game.start");
            return MenuAction::ResumeGame;
        }
        // if in game, render pause menu
        else if (gameState == GameState::InGame) {
            gameState = GameState::Pause;
            // play UI button clicked sound
            audioManager->jsonSound("ui.menu.button");
            return MenuAction::None;
        }
    }
    // triggers main menu (do not allow keyboard input to navigate to main menu while in game)
    else if (inputManager->isKeyPressedOnce(GLFW_KEY_M) && gameState != GameState::InGame) {
        // if on main menu, then start game
        if (gameState == GameState::MainMenu) {
            // play an "entering game" sound when button clicked
            audioManager->jsonSound("ui.game.start");
            return MenuAction::StartGame;
        }
        else if (gameState == GameState::Pause) {
            // play UI button clicked sound
            audioManager->jsonSound("ui.menu.button");
            return MenuAction::GoToMainMenu;
        }
        else if (gameState == GameState::GameOver) {
            // play UI button clicked sound
            audioManager->jsonSound("ui.menu.button");
            return MenuAction::GoToMainMenu;
        }
        else if (gameState == GameState::HelpMenu) {
            // play UI button clicked sound
            audioManager->jsonSound("ui.menu.button");
            return MenuAction::GoToMainMenu;
        }
        else if (gameState == GameState::ControllerHelp) {
            // play UI button clicked sound
            audioManager->jsonSound("ui.menu.button");
            return MenuAction::GoToMainMenu;
        }
        else if (gameState == GameState::KeyboardHelp) {
            // play UI button clicked sound
            audioManager->jsonSound("ui.menu.button");
            return MenuAction::GoToMainMenu;
        }
    }
    // triggers help menu (can only be navigated to from main menu)
    else if (inputManager->isKeyPressedOnce(GLFW_KEY_H) && gameState == GameState::MainMenu) {
        audioManager->jsonSound("ui.menu.button");
        gameState = GameState::HelpMenu; // update game state to render help page
        return MenuAction::None; // no action taken
    }
    // triggers controller help menu (can only be navigated to from Help menu)
    else if (inputManager->isKeyPressedOnce(GLFW_KEY_J) && gameState == GameState::HelpMenu) {
        audioManager->jsonSound("ui.menu.button");
        gameState = GameState::ControllerHelp; // update game state to render help page
        return MenuAction::None; // no action taken
    }
    // triggers keyboard help menu (can only be navigated to from Help menu)
    else if (inputManager->isKeyPressedOnce(GLFW_KEY_K) && gameState == GameState::HelpMenu) {
        audioManager->jsonSound("ui.menu.button");
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

    // get scaling factor
    glm::vec2 scalingFactor = getScale();

    // always take the smaller scaling factor
    float finalScale = std::min(scalingFactor.x, scalingFactor.y);

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    // create vao to draw menu background
    GPU_Geometry gpuQuadBackground;
    gpuQuadBackground.Update2D(quad); // update it with our basic quad info

    shader->use();

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    backgroundTexture->bind();
    glUniform1i(glGetUniformLocation(*shader, "sample"), 0);

    // static model to pass to shader, renders png as is
    glm::mat4 backgroundModel = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.f, 1.0f
    );

    // base matrix
    glm::mat4 backgroundProj = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    // pass matrices to shaders
    glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, glm::value_ptr(backgroundModel));
    glUniformMatrix4fv(glGetUniformLocation(*shader, "projection"), 1, GL_FALSE, glm::value_ptr(backgroundProj));

    // bind vao
    gpuQuadBackground.bind();

    // draw quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

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
    glm::mat4 logoModel = glm::mat4(
        finalScale, 0.0f, 0.0f, 0.0f,
        0.0f, finalScale, 0.0f, 0.0f,
        0.f, 0.0f, 1.0f, 0.0f,
        0.0f, translateY, 0.f, 1.0f
    );

    // get aspect ratio of current window size, cast to float since they are ints
    float aspectRatio = window->getAspectRatio();

    // orthogonal projection for logo to keep it from stretching past its' original size ratio
    glm::mat4 logoProj = glm::mat4(
        1.0f / aspectRatio, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    // pass matrices to shader
    glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, glm::value_ptr(logoModel));
    glUniformMatrix4fv(glGetUniformLocation(*shader, "projection"), 1, GL_FALSE, glm::value_ptr(logoProj));

    // bind vao
    gpuQuad.bind();

    // draw quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // default color for the "Start" button
    glm::vec3 defaultColor = blue;

    // set the "Start" button to the default color (used when not hovered upon)
    glm::vec3 startColor = defaultColor;

    // set the "Help" button to the default color (used when not hovered upon)
    glm::vec3 helpColor = defaultColor;

    // if detected controller is source of input, then auto select (highlight in red) first menu option
    if (inputSystem == 1) {
        // add "deadzone" (since most controllers won't return back to 0.0 exactly)
        float deadZone = 0.2f;

        // check for left stick movement to scroll through menu options
        // if negative, it is scrolling up
        // if positive, it is scrolling down
        if (inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_LEFT_Y) < -deadZone) {
            selectedMenuOption = 0; // if scrolled up, "start" button selected. save state
        }
        else if (inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_LEFT_Y) > deadZone) {
            selectedMenuOption = 1; // if scrolled down, "help" button selected. save state
        }

        // update button highlights based on which button currently selected (know from state keeping)
        if (selectedMenuOption == 0) {
            // select the "start" button
            startColor = red;
            helpColor = defaultColor;
        }
        else if (selectedMenuOption == 1) {
            // select the "help" button
            helpColor = red;
            startColor = defaultColor;
        }
    }

    // check if mouse is hovered over the "Start" button
    if (cursorPos.x > 440.f * scalingFactor.x && cursorPos.x < 635.f * scalingFactor.x) {
        if (cursorPos.y > 480.f * scalingFactor.y && cursorPos.y < 520.f * scalingFactor.y) {
            // if it is, highlight in red
            startColor = red;

            // and check if the user clicks on the mouse while over the "Start" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play an "entering game" sound when button clicked
                audioManager->jsonSound("ui.game.start");
                // if they do, toggle to NOT show the main menu
                return MenuAction::StartGame;
            }
        }
    }

    // check if mouse is hovered over the "Help" button
    if (cursorPos.x > 450.f * scalingFactor.x && cursorPos.x < 610.f * scalingFactor.x) {
        if (cursorPos.y > 555.f * scalingFactor.y && cursorPos.y < 595.f * scalingFactor.y) {
            // if it is, highlight in red
            helpColor = red;

            // and check if the user clicks on the mouse while over the "Help" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play ui button sound when button clicked
                audioManager->jsonSound("ui.menu.button");
                // if they do, toggle to show the help menu
                return MenuAction::GoToHelpMenu;
            }
        }
    }

    // render the text with the proper color assigned
    textSystem->renderText("Start", { 585.f, 400.f, 0.75f }, startColor);
    textSystem->renderText("Help", { 605.f, 250.f, 0.75f }, helpColor);

    // small text to credit ai generated background picture
    textSystem->loadFont("arial.ttf", 35);
    textSystem->renderText("Background image is generated by ChatGPT", { 900.f, 50.f, 0.75f }, defaultColor);

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

    // get scaling factor
    glm::vec2 scalingFactor = getScale();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    textSystem->renderText("PAUSE MENU", { 460.f, 1100.f, 0.75f }, { 0.f, 0.f, 0.55f });

    textSystem->loadFont("SNPro-SemiBold.ttf", 85);

    // default color for the buttons
    glm::vec3 defaultColor = blue;

    // set the buttons to the default color (used when not hovered upon)
    glm::vec3 resumeColor = defaultColor;
    glm::vec3 quitColor = defaultColor;

    // if detected controller is source of input, then auto select (highlight in red) first menu option
    if (inputSystem == 1) {
        // add "deadzone" (since most controllers won't return back to 0.0 exactly)
        float deadZone = 0.2f;

        // check for left stick movement to scroll through menu options
        // if negative, it is scrolling up
        // if positive, it is scrolling down
        if (inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_LEFT_Y) < -deadZone) {
            selectedPauseMenuOption = 0; // if scrolled up, "resume" button selected. save state
        }
        else if (inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_LEFT_Y) > deadZone) {
            selectedPauseMenuOption = 1; // if scrolled down, "quit" button selected. save state
        }

        // update button highlights based on which button currently selected (know from state keeping)
        if (selectedPauseMenuOption == 0) {
            // select the "resume" button
            resumeColor = red;
            quitColor = defaultColor;
        }
        else if (selectedPauseMenuOption == 1) {
            // select the "quit" button
            quitColor = red;
            resumeColor = defaultColor;
        }
    }

    // check if mouse is hovered over the "Resume" button
    if (cursorPos.x > 445.f * scalingFactor.x && cursorPos.x < 630.f * scalingFactor.x) {
        if (cursorPos.y > 240.f * scalingFactor.y && cursorPos.y < 270.f * scalingFactor.y) {
            // if it is, highlight in red
            resumeColor = red;

            // and check if the user clicks on the mouse while over the "Resume" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play an "entering game" sound when button clicked
                audioManager->jsonSound("ui.game.start");
                // if they do, resume the game
                return MenuAction::ResumeGame;
            }
        }
    }

    // check if mouse is hovered over the "Quit" button
    if (cursorPos.x > 275.f * scalingFactor.x && cursorPos.x < 845.f * scalingFactor.x) {
        if (cursorPos.y > 310.f * scalingFactor.y && cursorPos.y < 355.f * scalingFactor.y) {
            // if it is, highlight in red
            quitColor = red;

            // and check if the user clicks on the mouse while over the "Quit" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
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

    // get scaling factor
    glm::vec2 scalingFactor = getScale();

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

    if(engulfed) textSystem->renderText("ENGULFED !", {525.f, 600.f, 0.65f }, { 0.9f, 0.35f, 0.20f });

    // default color for the "Return to menu" button
    glm::vec3 defaultColor = blue;

    // set the "Return to menu" button to the default color (used when not hovered upon)
    glm::vec3 returnColor = defaultColor;

    // if detected controller is source of input, then auto select (highlight in red) first menu option
    if (inputSystem == 1) {
        returnColor = red;
    }

    // check for controller inputs
    if (inputSystem == 1) {
        // acknowledge game is over by clicking confirm (A)
        if (inputManager->isControllerButtonPressedOnce(GLFW_GAMEPAD_BUTTON_A)) {
            // play UI button clicked sound
            audioManager->jsonSound("ui.menu.button");
            // if "A" is pressed (bottom button?), go to main menu
            return MenuAction::GoToMainMenu;
        }
    }
    // otherwise it must be keyboard/mouse input
    // check if mouse is hovered over the "Return to menu" button
    if (cursorPos.x > 200.f * scalingFactor.x && cursorPos.x < 900.f * scalingFactor.x) {
        if (cursorPos.y > 630.f * scalingFactor.y && cursorPos.y < 675.f * scalingFactor.y) {
            // if it is, highlight in red
            returnColor = red;

            // and check if the user clicks on the mouse while over the "Return to menu" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play UI button clicked sound
                audioManager->jsonSound("ui.menu.button");
                // if they do, toggle to show the main menu
                return MenuAction::GoToMainMenu;
            }
        }
    }

    // return to main menu button
    textSystem->renderText("Return to Main Menu", { 270.f, 100.f, 0.75f }, returnColor);

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

    // get scaling factor
    glm::vec2 scalingFactor = getScale();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    // default color for all buttons
    glm::vec3 defaultColor = blue;

    // set the "Back" button to the default color (used when not hovered upon)
    glm::vec3 backColor = defaultColor;
    // set the "Keyboard" button to the default color (used when not hovered upon)
    glm::vec3 keyboardColor = defaultColor;
    // set the "Controller" button to the default color (used when not hovered upon)
    glm::vec3 controllerColor = defaultColor;

    // if detected controller is source of input, then auto select (highlight in red) first menu option
    if (inputSystem == 1) {
        // add "deadzone" (since most controllers won't return back to 0.0 exactly)
        float deadZone = 0.2f;

        // get the current (left) stick position in y-direction
        float currentStickPosition = inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_LEFT_Y);

        // check for left stick movement to scroll through menu options
        // if negative, it is scrolling up
        // if positive, it is scrolling down

        // if the stick currently is scrolling up, and the previous position was not scrolling up, then move to next menu option
        // (prevents hold scrolling which jumps past the middle option)
        if (currentStickPosition < -deadZone && stickPosition >= -deadZone) {
            if (selectedHelpMenuOption == 1) {
                selectedHelpMenuOption = 0; // scrolled up to "controller" from "keyboard" button
            }
            else if (selectedHelpMenuOption == 2) {
                selectedHelpMenuOption = 1; // scrolled up to "controller" from "back" button
            }
        }
        // if the stick currently is scrolling down, and the previous position was not scrolling down, then move to next menu option
        else if (currentStickPosition > deadZone && stickPosition <= deadZone) {
            if (selectedHelpMenuOption == 0) {
                selectedHelpMenuOption = 1; // scrolled down to "controller" from "keyboard" button
            }
            else if (selectedHelpMenuOption == 1) {
                selectedHelpMenuOption = 2; // scrolled down to "back" from "controller" button
            }
        }

        // update the latest stick position
        stickPosition = currentStickPosition;

        // update button highlights based on which button currently selected (know from state keeping)
        if (selectedHelpMenuOption == 0) {
            // select the "keyboard" button
            keyboardColor = red;
            backColor = defaultColor;
            controllerColor = defaultColor;
        }
        else if (selectedHelpMenuOption == 1) {
            // select the "controller" button
            controllerColor = red;
            keyboardColor = defaultColor;
            backColor = defaultColor;
        }
        else if (selectedHelpMenuOption == 2) {
            // select the "back" button
            backColor = red;
            keyboardColor = defaultColor;
            controllerColor = defaultColor;
        }
    }

    // check if mouse is hovered over the "Controller" button
    if (cursorPos.x > 340.f * scalingFactor.x && cursorPos.x < 730.f * scalingFactor.x) {
        if (cursorPos.y > 330.f * scalingFactor.y && cursorPos.y < 370.f * scalingFactor.y) {
            // if it is, highlight in red
            controllerColor = red;

            // and check if the user clicks on the mouse while over the "Controller" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play ui button sound when button clicked
                audioManager->jsonSound("ui.menu.button");
                // if they do, toggle to show the controller help menu
                return MenuAction::GoToControllerHelp;
            }
        }
    }
    // check if mouse is hovered over the "Keyboard" button
    if (cursorPos.x > 370.f * scalingFactor.x && cursorPos.x < 705.f * scalingFactor.x) {
        if (cursorPos.y > 230.f * scalingFactor.y && cursorPos.y < 270.f * scalingFactor.y) {
            // if it is, highlight in red
            keyboardColor = red;

            // and check if the user clicks on the mouse while over the "Keyboard" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play ui button sound when button clicked
                audioManager->jsonSound("ui.menu.button");
                // if they do, toggle to show the keyboard help menu
                return MenuAction::GoToKeyboardHelp;
            }
        }
    }
    // check if mouse is hovered over the "Back" button
    if (cursorPos.x > 445.f * scalingFactor.x && cursorPos.x < 610.f * scalingFactor.x) {
        if (cursorPos.y > 480.f * scalingFactor.y && cursorPos.y < 520.f * scalingFactor.y) {
            // if it is, highlight in red
            backColor = red;

            // and check if the user clicks on the mouse while over the "Back" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play ui button sound when button clicked
                audioManager->jsonSound("ui.menu.button");
                // if they do, toggle to show the main menu
                return MenuAction::GoToMainMenu;
            }
        }
    }

    // pages to choose from
    textSystem->renderText("Keyboard", { 500.f, 900.f, 0.75f }, keyboardColor);
    textSystem->renderText("Controller", { 460.f, 700.f, 0.75f }, controllerColor);

    // render the text with the proper color assigned
    textSystem->renderText("Back", { 600.f, 400.f, 0.75f }, backColor);

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

    // get scaling factor
    glm::vec2 scalingFactor = getScale();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    // default color for the "Return" button
    glm::vec3 defaultColor = blue;

    // set the "return" button to the default color (used when not hovered upon)
    glm::vec3 returnColor = defaultColor;

    // if detected controller is source of input, then auto select (highlight in red) first menu option
    if (inputSystem == 1) {
        returnColor = red;
    }

    // check if mouse is hovered over the "return" button
    if (cursorPos.x > 200.f * scalingFactor.x && cursorPos.x < 900.f * scalingFactor.x) {
        if (cursorPos.y > 630.f * scalingFactor.y && cursorPos.y < 675.f * scalingFactor.y) {
            // if it is, highlight in red
            returnColor = red;

            // and check if the user clicks on the mouse while over the "return" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play ui button sound when button clicked
                audioManager->jsonSound("ui.menu.button");
                // if they do, toggle to show the main menu
                return MenuAction::GoToMainMenu;
            }
        }
    }

    // create vao to draw image
    GPU_Geometry gpuQuad;
    gpuQuad.Update2D(quad); // update it with our basic quad info

    shader->use();

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    controllerTexture->bind();
    glUniform1i(glGetUniformLocation(*shader, "sample"), 0);

    // static model to pass to shader, renders png as is
    glm::mat4 model = glm::mat4(
        0.8f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.8f, 0.0f, 0.0f,
        0.f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.15f, 0.f, 1.0f
    );

    // base matrix
    glm::mat4 proj = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(*shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

    // bind vao
    gpuQuad.bind();

    // draw quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // return to main menu button
    textSystem->renderText("Return to Main Menu", { 270.f, 100.f, 0.75f }, returnColor);

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

    // get scaling factor
    glm::vec2 scalingFactor = getScale();

    textSystem->beginText();

    textSystem->loadFont("LuckiestGuy-Regular.ttf", 120);

    // default color for the "Return" button
    glm::vec3 defaultColor = blue;

    // set the "return" button to the default color (used when not hovered upon)
    glm::vec3 returnColor = defaultColor;

    // if detected controller is source of input, then auto select (highlight in red) first menu option
    if (inputSystem == 1) {
        returnColor = red;
    }

    // check if mouse is hovered over the "return" button
    if (cursorPos.x > 200.f * scalingFactor.x && cursorPos.x < 900.f * scalingFactor.x) {
        if (cursorPos.y > 630.f * scalingFactor.y && cursorPos.y < 675.f * scalingFactor.y) {
            // if it is, highlight in red
            returnColor = red;

            // and check if the user clicks on the mouse while over the "return" button
            if (inputManager->isMousePressedOnce(GLFW_MOUSE_BUTTON_LEFT)) {
                // play ui button sound when button clicked
                audioManager->jsonSound("ui.menu.button");
                // if they do, toggle to show the main menu
                return MenuAction::GoToMainMenu;
            }
        }
    }

    // create vao to draw image
    GPU_Geometry gpuQuad;
    gpuQuad.Update2D(quad); // update it with our basic quad info

    shader->use();

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    keyboardTexture->bind();
    glUniform1i(glGetUniformLocation(*shader, "sample"), 0);

    // static model to pass to shader, renders png as is
    glm::mat4 model = glm::mat4(
        0.8f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.8f, 0.0f, 0.0f,
        0.f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.15f, 0.f, 1.0f
    );

    // base matrix
    glm::mat4 proj = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(*shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

    // bind vao
    gpuQuad.bind();

    // draw quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // return to main menu button
    textSystem->renderText("Return to Main Menu", { 270.f, 100.f, 0.75f }, returnColor);

    textSystem->endText();

    // default return
    return MenuAction::None;
}

glm::vec2 GameMenus::getScale() {
    // initialize variables
    float scaleX = 1.f;
    float scaleY = 1.f;

    // calculate scaling factor based on current windopw size and default window size
    scaleX = window->getWidth() / defaultWindowWidth;
    scaleY = window->getHeight() / defaultWindowHeight;

    // return value
    glm::vec2 scale = {
        scaleX,
        scaleY
    };

    // return calculated value
    return scale;
}
