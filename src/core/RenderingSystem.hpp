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

    Entity createSphereEntity(const std::string& texturePath);
    Entity createModelEntity(const std::string& modelPath);
    Entity createSkyboxEntity(const std::string& texturePath);

    Renderable getCubeRenderable(const std::string& texturePath);
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
