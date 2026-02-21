#pragma once

#include "core/assets/Texture.hpp"
#include "core/assets/ModelLoader.hpp"
#include "core/buffer/Geometry.hpp"
#include "core/render/ShaderProgram.hpp"
#include "core/scene/TurnTableCamera.hpp"
#include "core/scene/FreeCamera.hpp"
#include "core/scene/Transform.hpp"

#include "components/Entity.h"
#include "components/Renderable.h"

#include "ecs/Coordinator.hpp"
#include "ecs/System.hpp"

#include "input/glfw/InputManager.hpp"
#include "input/glfw/Window.hpp"
#include "input/panel/ImGuiPanel.hpp"
#include "input/panel/ImGuiWrapper.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <vector>

extern Coordinator gCoordinator;

class RenderingSystem : public System {
public:
    RenderingSystem(std::shared_ptr<InputManager> inputManager, std::shared_ptr<Window> window, std::shared_ptr<ImGuiWrapper> imguiWrapper, std::shared_ptr<ImGuiPanel> imguiPanel);
    //~RenderingSystem();
    void cleanup();

    void update(float deltaTime);
    void updateUI();
    void endFrame();
    bool shouldClose() const;

    Entity createSphereEntity();
    Entity createModelEntity(const std::string& modelPath);
    Entity createSkyboxEntity();
    std::unique_ptr<Texture> texture2;
    std::unique_ptr<Texture> texture_snowball;
    std::unique_ptr<Texture> vehicleTexture;

    Renderable getCubeRenderable();
    void updateCameraTarget(const glm::vec3& position);
    glm::vec3 getCameraForward() const;
    bool isTurnTableCamera() { return activeCamera == turntableCamera.get();};

    // For rendering physics entities
    void renderEntities(const std::vector<EntityPx>& entityList);

    //inputManager getter
    std::shared_ptr<InputManager> getInputManager() const { return inputManager; }

    int getWindowWidth() const { return window->getWidth(); }
    int getWindowHeight() const { return window->getHeight(); }

    void onResize(int width, int height);
    void onMouseWheelChange(double xOffset, double yOffset);
    bool init();

private:
    // Core components following modular architecture
    std::shared_ptr<Window> window;
    std::unique_ptr<ShaderProgram> shader;
    std::unique_ptr<ShaderProgram> modelShader;
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
    
    // Skybox geometry (large inverted sphere)
    std::unique_ptr<GPU_Geometry> skyboxGeometry;
    std::unique_ptr<CPU_Geometry> skyboxCPUData;
    
    // Textures
    std::unique_ptr<Texture> texture;
    std::unique_ptr<Texture> skyboxTexture;
    
    // ImGui management (separated concerns)
    std::shared_ptr<ImGuiWrapper> imguiWrapper;  // Handles lifecycle
    std::shared_ptr<ImGuiPanel> imguiPanel;       // Handles content
    
    // Input management
    std::shared_ptr<InputManager> inputManager;
    glm::dvec2 previousCursorPosition{};
    bool cursorPositionIsSetOnce = false;

    void processInput(float deltaTime);
    void processCameraInput(float deltaTime);

    void toggleCamera();
    void render();
    glm::mat4 getProjectionMatrix() const;
};
