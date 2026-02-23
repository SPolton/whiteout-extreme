#pragma once

#include "core/assets/AssetManager.hpp"
#include "core/assets/ModelLoader.hpp"
#include "core/assets/Texture.hpp"
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
    RenderingSystem(std::shared_ptr<InputManager> inputManager);
    //~RenderingSystem();
    void cleanup();

    void update(float deltaTime);

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
    CameraStats getActiveCameraStats() { return activeCamera->getStats(); };

    // Parameters changed by Imgui in the syncImgui method of RacingGame
    float camSpeed = 1.0f;
    float camZoomSpeed = 1.0f;

    // Parameters window dimension resized on callback
    int vWidth;
    int vHeight;

    // For rendering physics entities
    void renderEntities(const std::vector<EntityPx>& entityList);

    void onMouseWheelChange(double xOffset, double yOffset);
    bool init();

private:
    AssetManager& assetManager = AssetManager::getInstance();

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
