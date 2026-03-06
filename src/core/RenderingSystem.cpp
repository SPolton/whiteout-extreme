#include "RenderingSystem.hpp"
#include "utils/logger.h"
#include "core/render/ShapeGenerator.hpp"

#include "ecs/Coordinator.hpp"
#include "components/Transform.h"
#include "components/VehicleComponent.h"

#include <iostream>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

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
        // TurnTableCamera uses right-click drag
        
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

    // Create common geometry through AssetManager
    CPU_Geometry sphereCPU = ShapeGenerator::sphere(1, 16, 16);
    assetManager.loadGeometry("sphere", sphereCPU);
    logger::info("Sphere geometry initialized");

    CPU_Geometry cubeCPU = ShapeGenerator::cube();
    assetManager.loadGeometry("cube", cubeCPU);
    logger::info("Cube geometry initialized");

    CPU_Geometry skyboxCPU = ShapeGenerator::sphere(100.0f, 32, 32);
    assetManager.loadGeometry("skybox", skyboxCPU);
    logger::info("Skybox geometry initialized");

    CPU_Geometry planeCPU = ShapeGenerator::plane(1.0f);
    assetManager.loadGeometry("plane", planeCPU);
    logger::info("Plane geometry initialized");

    // Create infinite ground plane with repeating texture (10000 units, 500 UV repeats)
    CPU_Geometry infinitePlaneCPU = ShapeGenerator::infinitePlane(10000.0f, 500.0f);
    assetManager.loadGeometry("infinite_plane", infinitePlaneCPU);
    logger::info("Infinite plane geometry initialized");

    // Create object tracking transform for camera (vehicle tracking)
    targetTransform = std::make_unique<SceneTransform>();
    targetTransform->setPosition(glm::vec3(0.f, 0.f, 0.f));

    // Create cameras
    turntableCamera = std::make_unique<TurnTableCamera>(*targetTransform);
    turntableCamera->adjustTheta(glm::radians(180.f));
    freeCamera = std::make_unique<FreeCamera>();
    activeCamera = turntableCamera.get();  // Non-owning raw pointer to turntable camera
    
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

Entity RenderingSystem::createSkyboxEntity(const std::string& texturePath)
{
    // Create skybox entity
    Entity skybox = gCoordinator.CreateEntity();

    gCoordinator.AddComponent(
        skybox,
        PhysxTransform{
            glm::vec3(0.f, 0.f, 0.f),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec3(1.f)
        }
    );

    gCoordinator.AddComponent(
        skybox,
        Renderable{
            .geometry = assetManager.loadGeometry("skybox", ShapeGenerator::sphere(100.0f, 32, 32)),
            .cpuData = assetManager.getCPUGeometry("skybox"),
            .shader = assetManager.loadShader("textured"),
            .texture = assetManager.loadTexture(texturePath, GL_LINEAR),
            .isSkybox = true
        }
    );

    logger::info("Skybox entity created");

    return skybox;
}

Entity RenderingSystem::createGroundPlaneEntity(const std::string& texturePath, float size)
{
    Entity groundPlane = gCoordinator.CreateEntity();

    bool isInfinite = (size <= 0.0f);
    
    if (isInfinite) {
        // Create infinite ground plane with repeating texture
        const float infiniteSize = 10000.0f;
        const float uvRepeat = 500.0f;
        
        gCoordinator.AddComponent(
            groundPlane,
            PhysxTransform{
                glm::vec3(0.f, 0.f, 0.f),  // Centered at origin
                glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                glm::vec3(1.f, 1.f, 1.f)  // No scaling needed since geometry is already large
            }
        );

        gCoordinator.AddComponent(
            groundPlane,
            Renderable{
                .geometry = assetManager.loadGeometry("infinite_plane", ShapeGenerator::infinitePlane(infiniteSize, uvRepeat)),
                .cpuData = assetManager.getCPUGeometry("infinite_plane"),
                .shader = assetManager.loadShader("textured"),
                .texture = assetManager.loadTexture(texturePath, GL_LINEAR, GL_REPEAT)  // Use GL_REPEAT for tiling
            }
        );

        logger::info("Infinite ground plane entity created with repeating texture");
    }
    else {
        // Create normal sized plane
        gCoordinator.AddComponent(
            groundPlane,
            PhysxTransform{
                glm::vec3(0.f, 0.f, 0.f),  // Centered at origin
                glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                glm::vec3(size, 1.f, size)  // Scale plane to requested size
            }
        );

        gCoordinator.AddComponent(
            groundPlane,
            Renderable{
                .geometry = assetManager.loadGeometry("plane", ShapeGenerator::plane(1.0f)),
                .cpuData = assetManager.getCPUGeometry("plane"),
                .shader = assetManager.loadShader("textured"),
                .texture = assetManager.loadTexture(texturePath, GL_LINEAR, GL_CLAMP_TO_EDGE)  // Clamp for normal plane
            }
        );

        logger::info("Ground plane entity created with size: {}", size);
    }

    return groundPlane;
}

//Create Entity and add PhysxTransform and Renderable Components
Entity RenderingSystem::createSphereEntity(const std::string& texturePath)
{
    // Create sphere entity
    Entity sphere = gCoordinator.CreateEntity();

    gCoordinator.AddComponent(
        sphere,
        PhysxTransform{
            glm::vec3(0.f, 0.f, 0.f),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec3(1.f)
        }
    );

    gCoordinator.AddComponent(
        sphere,
        Renderable{
            assetManager.loadGeometry("sphere", ShapeGenerator::sphere(1, 16, 16)),
            assetManager.getCPUGeometry("sphere"),
            assetManager.loadShader("textured"),
            assetManager.loadTexture(texturePath, GL_LINEAR)
        }
    );

    logger::info("Sphere entity created with texture: {}", texturePath);

    return sphere;
}

Entity RenderingSystem::createModelEntity(const std::string& modelPath)
{
    // Create model entity
    Entity model = gCoordinator.CreateEntity();

    gCoordinator.AddComponent(
        model,
        PhysxTransform{
            glm::vec3(0.f, 0.f, 0.f),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec3(1.f)
        }
    );

    // Create a new ModelLoader for this entity
    try {
        auto modelLoader = std::make_shared<ModelLoader>(modelPath, false);
        logger::info("Model loaded successfully: {} with {} meshes", modelPath, modelLoader->getMeshCount());
        
        // Add ModelRenderable component with the model loader and shader
        gCoordinator.AddComponent(
            model,
            ModelRenderable{modelLoader, assetManager.loadShader("model")}
        );
    }
    catch (const std::exception& e) {
        logger::error("Failed to load model {}: {}", modelPath, e.what());
        // Clean up the entity if model loading failed
        gCoordinator.DestroyEntity(model);
        throw;
    }

    logger::info("Model entity created: {}", modelPath);

    return model;
}

void RenderingSystem::render()
{
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //auto& camTransform = gCoordinator.GetComponent<PhysxTransform>(cameraEntity);
    //auto& camComp = gCoordinator.GetComponent<CameraComponent>(cameraEntity);

    glm::mat4 view = activeCamera->getViewMatrix();
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
                    renderable.cpuData->indices.size(),
                    GL_UNSIGNED_INT,
                    nullptr
                );
            }
            else {
                glDrawArrays(
                    GL_TRIANGLES,
                    0,
                    renderable.cpuData->positions.size()
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
                    1, GL_FALSE, &modelMatrix[0][0]
                );

                // Set lighting uniforms for the model shader
                glm::vec3 lightPos(0.0f, 20.0f, 0.0f);
                glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
                glm::vec3 viewPos = activeCamera->getPosition();
                
                glUniform3fv(glGetUniformLocation(*modelRenderable.shader, "lightPos"), 1, &lightPos[0]);
                glUniform3fv(glGetUniformLocation(*modelRenderable.shader, "lightColor"), 1, &lightColor[0]);
                glUniform3fv(glGetUniformLocation(*modelRenderable.shader, "viewPos"), 1, &viewPos[0]);

                // Draw the model (handles multiple meshes internally)
                modelRenderable.modelLoader->draw(*modelRenderable.shader);
            }
        }
    }
}

void RenderingSystem::renderEntities(const std::vector<EntityPx>& entityList)
{
    // Get shader and geometry
    auto shader = assetManager.loadShader("textured");
    auto cubeGeometry = assetManager.loadGeometry("cube", ShapeGenerator::cube());
    auto cubeCPUData = assetManager.getCPUGeometry("cube");
    
    // Use shader
    shader->use();
    
    // Get projection matrix (perspective projection for 3D)
    glm::mat4 projection = getProjectionMatrix();
    glUniformMatrix4fv(glGetUniformLocation(*shader, "projection"), 1, GL_FALSE, &projection[0][0]);
    
    // Get view matrix from active camera (transforms world coords to camera/view space)
    glm::mat4 view = activeCamera->getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(*shader, "view"), 1, GL_FALSE, &view[0][0]);
    
    // Bind cube geometry once
    cubeGeometry->bind();
    
    // Render each entity
    for (size_t i = 0; i < entityList.size(); i++) {
        glm::vec3 pos = entityList[i].transform->pos;
        glm::quat rot = entityList[i].transform->rot;
        
        // Model matrix: Scale -> Rotate -> Translate (SRT)
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);       // Translate
        model = model * glm::mat4_cast(rot);      // Rotate
        model = glm::scale(model, glm::vec3(1.0f)); // Scale
        
        // Send model matrix to shader
        glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, &model[0][0]);
        
        if (cubeCPUData) {
            // Draw the cube
            if (!cubeCPUData->indices.empty()) {
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cubeCPUData->indices.size()), GL_UNSIGNED_INT, nullptr);
            }
            else {
                glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cubeCPUData->positions.size()));
            }
        }
    }
}

glm::mat4 RenderingSystem::getProjectionMatrix() const
{
    float const aspectRatio = static_cast<float>(vWidth) / static_cast<float>(vHeight);
    
    // Perspective projection for active camera
    // FOV is already in radians, no conversion needed
    return glm::perspective(activeCamera->getFOV(), aspectRatio, 0.1f, 5000.0f);
}

void RenderingSystem::update(float deltaTime)
{
    processInput(deltaTime);

    // center skybox on camera
    for (auto const& entity : mEntities) {
        if (gCoordinator.HasComponent<Renderable>(entity)) {
            auto& renderable = gCoordinator.GetComponent<Renderable>(entity);
            if (renderable.isSkybox) {
                auto& transform = gCoordinator.GetComponent<PhysxTransform>(entity);
                transform.pos = activeCamera->getPosition();
            }
        }
    }
    
    // Render the rotating sphere (demo)
    render();
}

void RenderingSystem::onMouseWheelChange(double xOffset, double yOffset)
{
    float scroll = -static_cast<float>(yOffset) * this->camZoomSpeed * 0.016f;
    activeCamera->adjustRadius(scroll);
}

void RenderingSystem::toggleCamera()
{
    if (activeCamera == turntableCamera.get())
    {
        activeCamera = freeCamera.get();
        logger::info("Switched to FreeCamera (FPS-style)");
    }
    else
    {
        activeCamera = turntableCamera.get();
        logger::info("Switched to TurnTableCamera (Orbit)");
    }
}

void RenderingSystem::updateCameraTarget(const glm::vec3& position)
{
    if (targetTransform) {
        targetTransform->setPosition(position);
    }
}

glm::vec3 RenderingSystem::getCameraForward() const
{
    auto view = activeCamera->getViewMatrix();
    return -glm::vec3(view[0][2], view[1][2], view[2][2]);
}
