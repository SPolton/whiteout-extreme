#pragma once

#include "core/assets/AssetManager.hpp"
#include "core/assets/ModelLoader.hpp"
#include "core/assets/Texture.hpp"
#include "core/buffer/Geometry.hpp"
#include "core/render/ShaderProgram.hpp"
#include "core/render/SnowRenderer.hpp"
#include "core/render/ShapeConfig.hpp"
#include "core/scene/TurnTableCamera.hpp"
#include "core/scene/FreeCamera.hpp"
#include "core/scene/RacingCamera.hpp"
#include "core/scene/Transform.hpp"

#include "components/Model.h"
#include "components/Renderable.h"

#include "ecs/Coordinator.hpp"
#include "ecs/System.hpp"

#include "input/glfw/InputManager.hpp"
#include "input/glfw/Window.hpp"
#include "input/panel/ImGuiPanel.hpp"
#include "input/panel/ImGuiWrapper.hpp"
#include "vfx/SnowParticle.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

extern Coordinator gCoordinator;

struct RenderingStats {
    bool isEnabled = false;
    float frameMs = 0.0f;
    float geometryMs = 0.0f;
    float modelMs = 0.0f;
    float particlesMs = 0.0f;
    uint32_t renderableDrawCalls = 0;
    uint32_t modelDrawCalls = 0;

    std::string toString() const;

    void startFrame();
    void startGeometryPass();
    void endGeometryPass();
    void startModelPass();
    void endModelPass();
    void startParticlePass();
    void endParticlePass();
    void endFrame();
    void addRenderableDraw() { if (isEnabled) { ++renderableDrawCalls; } }
    void addModelDraw() { if (isEnabled) { ++modelDrawCalls; } }
private:
    std::chrono::steady_clock::time_point geometryStart{};
    std::chrono::steady_clock::time_point modelStart{};
    std::chrono::steady_clock::time_point particlesStart{};
    bool geometryPassActive = false;
    bool modelPassActive = false;
    bool particlePassActive = false;
};

class RenderingSystem : public System {
public:
    RenderingSystem(std::shared_ptr<InputManager> inputManager);
    void cleanup();

    void update(float deltaTime);

    // Flexible entity creation methods
    Entity createBoxEntity(const std::string& texturePath, const render::BoxConfig& config = {});
    Entity createSphereEntity(const std::string& texturePath, const render::SphereConfig& config = {});
    Entity createPlaneEntity(const std::string& texturePath, const render::PlaneConfig& config = {});
    Entity createModelEntity(const std::string& modelPath, const render::ModelConfig& config = {});

    Renderable getCubeRenderable(const std::string& texturePath);

    void updateCameraTarget(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& velocity);
    void setSnowFrame(const SnowFrame& frame);

    void enableStats(bool isEnabled) { statsData.isEnabled = isEnabled; }
    const RenderingStats& renderStats() const { return statsData; }

    glm::vec3 getCameraForward() const { return activeCamera->forward(); };
    glm::vec3 getCameraRight() const { return activeCamera->right(); };
    std::string getActiveCameraInfo() const { return activeCamera->toString(); };

    // Parameters changed by Imgui in the syncImgui method of RacingGame
    float camSpeed = 1.0f;
    float camZoomSpeed = 1.0f;

    // Parameters window dimension resized on callback
    int vWidth;
    int vHeight;

    void onMouseWheelChange(double xOffset, double yOffset);
    bool init();

private:
    RenderingStats statsData{};
    AssetManager& assetManager = AssetManager::getInstance();
    SnowRenderer snowRenderer;

    std::unique_ptr<FreeCamera> freeCamera;
    std::unique_ptr<RacingCamera> racingCamera;
    std::unique_ptr<TurnTableCamera> turntableCamera;
    BaseCamera* activeCamera;  // Pointer to the currently active camera

    std::unique_ptr<SceneTransform> targetTransform; // Camera target
    
    // Input management
    std::shared_ptr<InputManager> inputManager;
    glm::dvec2 previousCursorPosition{};
    bool cursorPositionIsSetOnce = false;
    float lastFrameDeltaTime = 1.0f / 60.0f;

    void processInput(float deltaTime);
    void processCameraInput(float deltaTime);
    void updateSkyboxFollow();

    void renderRenderableEntity(Entity entity, const glm::mat4& view, const glm::mat4& projection);
    void renderModelEntity(Entity entity, const glm::mat4& view, const glm::mat4& projection);
    void renderParticles(const glm::mat4& view, const glm::mat4& projection);

    void toggleCamera();
    void render();
    glm::mat4 getProjectionMatrix() const;
};
