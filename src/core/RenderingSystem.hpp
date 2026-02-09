#pragma once

#include "input/Window.hpp"
#include "core/assets/Texture.hpp"
#include "core/buffer/Geometry.hpp"
#include "core/render/ShaderProgram.hpp"
#include "core/scene/TurnTableCamera.hpp"
#include "core/scene/FreeCamera.hpp"
#include "core/scene/Transform.hpp"
#include "input/panel/ImGuiWrapper.hpp"
#include "input/panel/ImGuiPanel.hpp"
#include "input/InputManager.hpp"
#include "components/Entity.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <vector>

class RenderingSystem {
public:
    RenderingSystem();
    ~RenderingSystem();

    void update(float deltaTime);
    void updateUI();
    void endFrame();
    bool shouldClose() const;

    // For rendering physics entities
    void renderEntities(const std::vector<Entity>& entityList);

private:
    // Core components following modular architecture
    std::unique_ptr<Window> window;
    std::unique_ptr<ShaderProgram> shader;
    std::unique_ptr<TurnTableCamera> turntableCamera;
    std::unique_ptr<FreeCamera> freeCamera;
    BaseCamera* activeCamera;  // Pointer to the currently active camera
    
    std::unique_ptr<SceneTransform> targetTransform; // Camera target
    
    // Geometry using RAII wrappers
    std::unique_ptr<GPU_Geometry> triangleGeometry;
    std::unique_ptr<CPU_Geometry> triangleCPUData;
    
    // Cube geometry for physics objects
    std::unique_ptr<GPU_Geometry> cubeGeometry;
    std::unique_ptr<CPU_Geometry> cubeCPUData;
    
    // Textures
    std::unique_ptr<Texture> texture;
    std::unique_ptr<Texture> vehicleTexture;
    
    // ImGui management (separated concerns)
    std::unique_ptr<ImGuiWrapper> imguiWrapper;  // Handles lifecycle
    std::unique_ptr<ImGuiPanel> imguiPanel;       // Handles content
    
    // Input management
    std::shared_ptr<InputManager> inputManager;
    glm::dvec2 previousCursorPosition{};
    bool cursorPositionIsSetOnce = false;

    // Basic Movement
    void accelerate();
    void brake();
    void steerRight();
    void steerLeft();

    // Skills
    void boost();
    void throwSnowball();
    
    bool init();
    void processInput(float deltaTime);
    void processKeyboardInput();
    void processControllerInput();
    void processCameraInput(float deltaTime);

    void updateCameraTarget(const glm::vec3& position);
    void toggleCamera();
    void render();
    void onResize(int width, int height);
    void onMouseWheelChange(double xOffset, double yOffset);
    glm::mat4 getProjectionMatrix() const;

    // Pause/Menu
    bool isGamePaused = false;
    void gamePaused();
};
