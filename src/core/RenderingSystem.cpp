#include "RenderingSystem.hpp"
#include "utils/logger.h"
#include "core/render/ShapeGenerator.hpp"

#include "ecs/Coordinator.hpp"
#include "components/Transform.h"
#include "components/VehicleComponent.h"

#include <glm/gtc/type_ptr.hpp>

using namespace render;

RenderingSystem::RenderingSystem(
    std::shared_ptr<InputManager> inputManager)
    : inputManager(inputManager)
{
    if (!init()) {
        throw std::runtime_error("Failed to initialize RenderingSystem!");
    }
}

void RenderingSystem::processInput(float deltaTime)
{
    // Handle camera toggle with F key
    if (inputManager->isKeyPressedOnce(GLFW_KEY_F)) {
        toggleCamera();
    }

    processCameraInput(deltaTime);
}

// Camera Input Processing
void RenderingSystem::processCameraInput(float deltaTime)
{
    auto const cursorPosition = inputManager->cursorPosition();

    // Check if we're using FreeCamera
    if (activeCamera == freeCamera.get())
    {
        // FreeCamera uses mouse movement when right mouse button is held
        if (inputManager->isMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            if (cursorPositionIsSetOnce) {
                auto const deltaPosition = cursorPosition - previousCursorPosition;
                freeCamera->processMouseMovement(
                    static_cast<float>(deltaPosition.x),
                    static_cast<float>(-deltaPosition.y)  // Invert Y for natural feel
                );
            }
        }

        // IJKL/UO controls for FreeCamera movement (don't interfere with WASD game controls)
        if (inputManager->isKeyPressed(GLFW_KEY_I))
            freeCamera->processKeyboard(FreeCamera::Movement::FORWARD, deltaTime);
        if (inputManager->isKeyPressed(GLFW_KEY_K))
            freeCamera->processKeyboard(FreeCamera::Movement::BACKWARD, deltaTime);
        if (inputManager->isKeyPressed(GLFW_KEY_J))
            freeCamera->processKeyboard(FreeCamera::Movement::LEFT, deltaTime);
        if (inputManager->isKeyPressed(GLFW_KEY_L))
            freeCamera->processKeyboard(FreeCamera::Movement::RIGHT, deltaTime);
        if (inputManager->isKeyPressed(GLFW_KEY_U))
            freeCamera->processKeyboard(FreeCamera::Movement::UP, deltaTime);
        if (inputManager->isKeyPressed(GLFW_KEY_O))
            freeCamera->processKeyboard(FreeCamera::Movement::DOWN, deltaTime);
    }
    else if (activeCamera == turntableCamera.get())
    {
        // poll controller state first
        inputManager->pollControllerInputs();

        // TurnTableCamera uses right-click drag
        if (inputManager->isMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            if (cursorPositionIsSetOnce) {
                float const aspectRatio = static_cast<float>(vWidth) / static_cast<float>(vHeight);
                auto const deltaPosition = cursorPosition - previousCursorPosition;
                turntableCamera->adjustTheta(-static_cast<float>(deltaPosition.x) * deltaTime * this->camSpeed * (1 / aspectRatio));
                turntableCamera->adjustPhi(-static_cast<float>(deltaPosition.y) * deltaTime * this->camSpeed);
            }
        }
        else if (inputManager->isControllerConnected())
        {
            float rx = inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_RIGHT_X);
            float ry = inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_RIGHT_Y);

            const float deadzone = 0.20f;
            if (std::abs(rx) < deadzone) rx = 0.0f;
            if (std::abs(ry) < deadzone) ry = 0.0f;

            if (rx != 0.0f || ry != 0.0f) {
                float const aspectRatio = static_cast<float>(vWidth) / static_cast<float>(vHeight);
                float sensitivity = 2.0f;
                turntableCamera->adjustTheta(-rx * deltaTime * this->camSpeed * sensitivity * (1.0f / aspectRatio));
                turntableCamera->adjustPhi(-ry * deltaTime * this->camSpeed * sensitivity);
            }
        }
    }

    // Always update cursor position tracking
    cursorPositionIsSetOnce = true;
    previousCursorPosition = cursorPosition;
}

bool RenderingSystem::init()
{
    logger::info("OpenGL Version: {0}", (const char*)glGetString(GL_VERSION));
    logger::info("OpenGL Renderer: {0}", (const char*)glGetString(GL_RENDERER));

    try
    {
        assetManager.loadShader("textured");
        logger::info("Textured shader loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load textured shader: {0}", e.what());
        return false;
    }

    // Load model shader
    try
    {
        assetManager.loadShader("model");
        logger::info("Model shader loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load model shader: {0}", e.what());
        return false;
    }

    // Create object tracking transform for camera (vehicle tracking)
    targetTransform = std::make_unique<SceneTransform>();
    targetTransform->setPosition(glm::vec3(0.f, 0.f, 0.f));

    // Create cameras
    racingCamera = std::make_unique<RacingCamera>();
    turntableCamera = std::make_unique<TurnTableCamera>(*targetTransform);
    turntableCamera->adjustTheta(glm::radians(180.f));
    freeCamera = std::make_unique<FreeCamera>();
    activeCamera = racingCamera.get();  // Non-owning raw pointer to camera
    
    logger::info("Camera initialized");

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    return true;
}

Renderable RenderingSystem::getCubeRenderable(const std::string& texturePath)
{
    return Renderable{
        .geometry = assetManager.loadGeometry("cube", ShapeGenerator::cube()),
        .cpuData = assetManager.getCPUGeometry("cube"),
        .shader = assetManager.loadShader("textured"),
        .texture = assetManager.loadTexture(texturePath, GL_LINEAR)
    };
}

Entity RenderingSystem::createPlaneEntity(const std::string& texturePath, const PlaneConfig& config)
{
    Entity plane = gCoordinator.CreateEntity();

    gCoordinator.AddComponent(
        plane,
        PhysxTransform{
            config.position,
            config.rotation,
            config.scale
        }
    );

    std::string geomKey;
    CPU_Geometry planeCPU;
    
    if (config.isInfinite) {
        geomKey = "infinite_plane";
        planeCPU = ShapeGenerator::infinitePlane(config.infinitePlaneSize, config.uvRepeat);
        logger::info("Infinite plane entity created with UV repeat: {}", config.uvRepeat);
    } else {
        geomKey = "plane";
        planeCPU = ShapeGenerator::plane(config.size);
        logger::info("Plane entity created with size: {}", config.size);
    }

    gCoordinator.AddComponent(
        plane,
        Renderable{
            .geometry = assetManager.loadGeometry(geomKey, planeCPU),
            .cpuData = assetManager.getCPUGeometry(geomKey),
            .shader = assetManager.loadShader("textured"),
            .texture = assetManager.loadTexture(texturePath, config.textureFilterMode, config.textureWrapMode)
        }
    );

    return plane;
}

Entity RenderingSystem::createBoxEntity(const std::string& texturePath, const render::BoxConfig& config)
{
    Entity box = gCoordinator.CreateEntity();

    gCoordinator.AddComponent(
        box,
        PhysxTransform{
            config.position,
            config.rotation,
            config.scale
        }
    );

    gCoordinator.AddComponent(
        box,
        getCubeRenderable(texturePath)
    );

    logger::info("Box entity created at ({}, {}, {})", config.position.x, config.position.y, config.position.z);
    return box;
}

Entity RenderingSystem::createSphereEntity(const std::string& texturePath, const SphereConfig& config)
{
    Entity sphere = gCoordinator.CreateEntity();

    gCoordinator.AddComponent(
        sphere,
        PhysxTransform{
            config.position,
            config.rotation,
            config.scale
        }
    );

    // Generate geometry key based on config
    std::string geomKey = config.isSkybox ? "skybox" : "sphere";
    
    // Create geometry with specified parameters
    CPU_Geometry sphereCPU = ShapeGenerator::sphere(config.radius, config.slices, config.stacks);
    
    gCoordinator.AddComponent(
        sphere,
        Renderable{
            .geometry = assetManager.loadGeometry(geomKey, sphereCPU),
            .cpuData = assetManager.getCPUGeometry(geomKey),
            .shader = assetManager.loadShader("textured"),
            .texture = assetManager.loadTexture(texturePath, config.textureFilterMode, config.textureWrapMode),
            .isSkybox = config.isSkybox
        }
    );

    logger::info("Sphere entity created (radius={}, skybox={}) with texture: {}", 
                 config.radius, config.isSkybox, texturePath);

    return sphere;
}

Entity RenderingSystem::createModelEntity(const std::string& modelPath, const ModelConfig& config)
{
    Entity model = gCoordinator.CreateEntity();

    gCoordinator.AddComponent(
        model,
        PhysxTransform{
            config.position,
            config.rotation,
            config.scale
        }
    );

    try {
        auto modelLoader = std::make_shared<ModelLoader>(modelPath, false);
        logger::info("Model loaded successfully: {} with {} meshes", modelPath, modelLoader->getMeshCount());
        
        gCoordinator.AddComponent(
            model,
            ModelRenderable{modelLoader, assetManager.loadShader("model")}
        );
    }
    catch (const std::exception& e) {
        logger::error("Failed to load model {}: {}", modelPath, e.what());
        throw;
    }

    logger::info("Model entity created: {}", modelPath);

    return model;
}

void RenderingSystem::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = activeCamera->viewMatrix();
    glm::mat4 projection = getProjectionMatrix();

    // Iterate through all entities that the RenderingSystem tracks
    // (entities with Transform component)
    for (auto const& entity : mEntities)
    {
        auto& transform = gCoordinator.GetComponent<PhysxTransform>(entity);
        
        // Calculate model matrix from transform
        glm::mat4 modelMatrix =
            glm::translate(glm::mat4(1.f), transform.pos)
            * glm::toMat4(transform.rot)
            * glm::scale(glm::mat4(1.f), transform.scale);

        // Check if entity has a simple Renderable component (sphere, cube, etc.)
        if (gCoordinator.HasComponent<Renderable>(entity))
        {
            auto& renderable = gCoordinator.GetComponent<Renderable>(entity);

            renderable.updateRollingTexture(transform.pos);

            // Apply special rendering settings for skybox
            if (renderable.isSkybox) {
                glDepthMask(GL_FALSE);  // Don't write to depth buffer
                glFrontFace(GL_CW);     // Reverse winding order to see inside
            }

            glm::vec3 visualPos = transform.pos;

            // If entity has VehicleComponent, apply offset for red brick model rendering
            if (gCoordinator.HasComponent<VehicleComponent>(entity)) {
                glm::vec3 localOffset(0.0f, 0.75f, 2.0f);
                glm::vec3 rotatedOffset = transform.rot * localOffset;
                visualPos += rotatedOffset;
            }

            renderable.shader->use();

            glActiveTexture(GL_TEXTURE0);
            renderable.texture->bind();
            glUniform1i(
                glGetUniformLocation(*renderable.shader, "baseColorTexture"),
                0
            );

            // Pass texture scroll offsets to shader for rolling textures
            glUniform2fv(
                glGetUniformLocation(*renderable.shader, "textureScrollOffset"),
                1,
                glm::value_ptr(renderable.textureScrollOffset)
            );

            glm::mat4 model =
                glm::translate(glm::mat4(1.f), visualPos)
                * glm::toMat4(transform.rot)
                * glm::scale(glm::mat4(1.f), transform.scale);

            glUniformMatrix4fv(
                glGetUniformLocation(*renderable.shader, "model"),
                1, GL_FALSE, &model[0][0]
            );

            glUniformMatrix4fv(
                glGetUniformLocation(*renderable.shader, "view"),
                1, GL_FALSE, &view[0][0]
            );

            glUniformMatrix4fv(
                glGetUniformLocation(*renderable.shader, "projection"),
                1, GL_FALSE, &projection[0][0]
            );

            renderable.geometry->bind();

            if (!renderable.cpuData->indices.empty()) {
                glDrawElements(
                    GL_TRIANGLES,
                    static_cast<GLsizei>(renderable.cpuData->indices.size()),
                    GL_UNSIGNED_INT,
                    nullptr
                );
            }
            else {
                glDrawArrays(
                    GL_TRIANGLES,
                    0,
                    static_cast<GLsizei>(renderable.cpuData->positions.size())
                );
            }

            // Restore normal rendering state if this was skybox
            if (renderable.isSkybox) {
                glDepthMask(GL_TRUE);
                glFrontFace(GL_CCW);
            }
        }

        // Check if entity has a ModelRenderable component (complex 3D models)
        else if (gCoordinator.HasComponent<ModelRenderable>(entity))
        {
            auto& modelRenderable = gCoordinator.GetComponent<ModelRenderable>(entity);

            if (modelRenderable.modelLoader && modelRenderable.shader) {
                modelRenderable.shader->use();

                // --- COMPUTE NEW MATRIX WITH OFFSET FOR SNOWMOBILES ---
                glm::vec3 offsetInWorldSpace = transform.rot * modelRenderable.visualOffsetPos;

                glm::mat4 correctedModelMatrix =
                    glm::translate(glm::mat4(1.0f), transform.pos + offsetInWorldSpace)
                    * glm::toMat4(transform.rot)
                    * glm::scale(glm::mat4(1.0f), transform.scale);
                // ------------------------------------------

                // Set up view and projection matrices
                glUniformMatrix4fv(
                    glGetUniformLocation(*modelRenderable.shader, "view"),
                    1, GL_FALSE, &view[0][0]
                );

                glUniformMatrix4fv(
                    glGetUniformLocation(*modelRenderable.shader, "projection"),
                    1, GL_FALSE, &projection[0][0]
                );

                // Set model matrix using the entity's transform
                glUniformMatrix4fv(
                    glGetUniformLocation(*modelRenderable.shader, "model"),
                    1, GL_FALSE, &correctedModelMatrix[0][0]
                );

                // Set lighting uniforms for the model shader
                glm::vec3 lightPos(0.0f, 20.0f, 0.0f);
                glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
                glm::vec3 viewPos = activeCamera->position();
                
                glUniform3fv(glGetUniformLocation(*modelRenderable.shader, "lightPos"), 1, &lightPos[0]);
                glUniform3fv(glGetUniformLocation(*modelRenderable.shader, "lightColor"), 1, &lightColor[0]);
                glUniform3fv(glGetUniformLocation(*modelRenderable.shader, "viewPos"), 1, &viewPos[0]);

                // Draw the model (handles multiple meshes internally)
                modelRenderable.modelLoader->draw(*modelRenderable.shader);
            }
        }
    }
}

glm::mat4 RenderingSystem::getProjectionMatrix() const
{
    float const aspectRatio = static_cast<float>(vWidth) / static_cast<float>(vHeight);
    
    // Perspective projection for active camera
    // FOV is already in radians, no conversion needed
    return glm::perspective(activeCamera->fov(), aspectRatio, 0.1f, 5000.0f);
}

void RenderingSystem::update(float deltaTime)
{
    if (freeCamera) {
        freeCamera->movementSpeed(camSpeed * 10.f);
    }
    if (racingCamera) {
        racingCamera->update(deltaTime);
    }

    processInput(deltaTime);

    // center skybox on camera
    for (auto const& entity : mEntities) {
        if (gCoordinator.HasComponent<Renderable>(entity)) {
            auto& renderable = gCoordinator.GetComponent<Renderable>(entity);
            if (renderable.isSkybox) {
                auto& transform = gCoordinator.GetComponent<PhysxTransform>(entity);
                transform.pos = activeCamera->position();
            }
        }
    }
    
    render();
}

void RenderingSystem::onMouseWheelChange(double xOffset, double yOffset)
{
    (void)xOffset;
    float scroll = -static_cast<float>(yOffset) * this->camZoomSpeed * 0.016f;

    if (activeCamera == freeCamera.get()) {
        freeCamera->adjustFov(scroll);
    }
    else if (activeCamera == turntableCamera.get()) {
        turntableCamera->adjustDistance(scroll);
    }
}

void RenderingSystem::toggleCamera()
{
    if (activeCamera == freeCamera.get())
    {
        activeCamera = racingCamera.get();
        logger::info("Switched to RacingCamera");
    }
    else if (activeCamera == racingCamera.get())
    {
        activeCamera = turntableCamera.get();
        logger::info("Switched to TurnTableCamera (Orbit)");
    }
    else
    {
        freeCamera->position(activeCamera->position());
        activeCamera = freeCamera.get();
        logger::info("Switched to FreeCamera");
    }
}

void RenderingSystem::updateCameraTarget(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& velocity)
{
    if (targetTransform) {
        targetTransform->setPosition(position);
    }

    if (racingCamera) {
        racingCamera->updateTarget(position, forward, velocity);
    }
}
