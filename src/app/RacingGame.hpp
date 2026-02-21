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

    std::shared_ptr<InputManager> inputManager;
    std::shared_ptr<Window> window;

    std::shared_ptr<ImGuiWrapper> imguiWrapper;
    std::shared_ptr<ImGuiPanel> imguiPanel;
    void updateImGui() {
        glm::vec3 bgColor = imguiPanel->getBackgroundColor();
        glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);

        // Set viewport
        glViewport(0, 0, window->getWidth(), window->getHeight());

        // Apply wireframe mode if enabled
        if (imguiPanel->showWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // Update camera stats for UI
        imguiPanel->cameraStats = renderingSystem->getActiveCameraStats();
        imguiPanel->cameraStats.aspect = static_cast<float>(window->getWidth()) / static_cast<float>(window->getHeight());

        // Update UI
        imguiWrapper->beginFrame();
        imguiPanel->update();
        imguiWrapper->renderFPS();
        this->syncImgui();
        imguiPanel->cameraStats = renderingSystem->getActiveCameraStats();
        imguiWrapper->endFrame();
    };

    void syncImgui() {
        renderingSystem->camSpeed = imguiPanel->camSpeed;
        renderingSystem->camZoomSpeed = imguiPanel->camZoomSpeed;
        //renderingSystem->wireframeMode = imguiPanel->showWireframe;
    }

    void endFrame() {
        window->swapBuffers();
        glfwPollEvents();
    }


    std::shared_ptr<RenderingSystem> renderingSystem;
    std::shared_ptr<PhysicsSystem> physicsSystem;
    std::shared_ptr<VehicleControlSystem> vehicleControlSystem;

    std::unique_ptr<Text> textSystem;

    // access UI menus
    std::unique_ptr<GameMenus> menus;
    // set default game state on main menu
    GameState gameState = GameState::MainMenu;

    //Entity playerVehicleEntity;
    Entity Earth = 0;
    Entity Mars = 0;
    Entity WoodyModel = 0;
    Entity BackpackModel = 0;
    Entity MapModel = 0;
};
