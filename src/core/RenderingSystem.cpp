#include "RenderingSystem.hpp"
#include "utils/logger.h"
#include "core/render/ShapeGenerator.hpp"

#include <iostream>
#include <cmath>

#include "ecs/Coordinator.hpp"
#include "components/Transform.h"
#include "components/VehicleComponent.h"

RenderingSystem::RenderingSystem() {
    if (!init()) {
        throw std::runtime_error("Failed to initialize RenderingSystem!");
    }
}

/*
RenderingSystem::~RenderingSystem()
{
    // ImGui cleanup via wrapper
    if (imguiWrapper) {
        imguiWrapper->shutdown();
    }
}
*/

void RenderingSystem::processInput(float deltaTime)
{
    if (inputManager->isKeyPressedOnce(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window->getGLFWwindow(), true);

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
                float const aspectRatio = static_cast<float>(window->getWidth()) / static_cast<float>(window->getHeight());
                auto const deltaPosition = cursorPosition - previousCursorPosition;
                turntableCamera->adjustTheta(-static_cast<float>(deltaPosition.x) * deltaTime * imguiPanel->camSpeed * (1 / aspectRatio));
                turntableCamera->adjustPhi(-static_cast<float>(deltaPosition.y) * deltaTime * imguiPanel->camSpeed);
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
                float const aspectRatio = static_cast<float>(window->getWidth()) / static_cast<float>(window->getHeight());
                float sensitivity = 2.0f;
                turntableCamera->adjustTheta(-rx * deltaTime * imguiPanel->camSpeed * sensitivity * (1.0f / aspectRatio));
                turntableCamera->adjustPhi(-ry * deltaTime * imguiPanel->camSpeed * sensitivity);
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
    // Initialize GLFW (needs to be done before creating Window)
    if (!glfwInit())
    {
        logger::error("Failed to initialize GLFW");
        return false;
    }

    // Set OpenGL version hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window using our Window wrapper (RAII)
    window = std::make_unique<Window>(1200, 800, "Racing Game");
    window->makeContextCurrent();

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        logger::error("Failed to initialize GLAD");
        return false;
    }

    logger::info("OpenGL Version: {0}", (const char*)glGetString(GL_VERSION));
    logger::info("OpenGL Renderer: {0}", (const char*)glGetString(GL_RENDERER));

    // Initialize ImGui using wrapper (handles lifecycle)
    imguiWrapper = std::make_unique<ImGuiWrapper>();
    if (!imguiWrapper->init(window->getGLFWwindow()))
    {
        logger::error("Failed to initialize ImGui");
        return false;
    }
    
    // Create ImGui panel (handles content)
    imguiPanel = std::make_unique<ImGuiPanel>();
    logger::info("ImGui initialized");

    // Initialize input manager with callbacks
    inputManager = std::make_shared<InputManager>(
        [this](int const width, int const height)->void { onResize(width, height); },
        [this](double const xOffset, double const yOffset)->void { onMouseWheelChange(xOffset, yOffset); }
    );
    
    window->setCallbacks(inputManager);
    logger::info("Input manager initialized");

    // Create shader using ShaderProgram (RAII)
    try
    {
        shader = std::make_unique<ShaderProgram>(
            "assets/shaders/textured.vert", 
            "assets/shaders/textured.frag"
        );
        logger::info("Shaders loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load shaders: {0}", e.what());
        return false;
    }

    // Create model shader for loaded 3D models (different vertex layout)
    try
    {
        modelShader = std::make_unique<ShaderProgram>(
            "assets/shaders/model.vert",
            "assets/shaders/model.frag"
        );
        logger::info("Model shaders loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load model shaders: {0}", e.what());
        return false;
    }

    // Load texture
    try
    {
        texture = std::make_unique<Texture>(
            "assets/textures/2k_earth_daymap.jpg",
            GL_LINEAR
        );
        texture2 = std::make_unique<Texture>(
            "assets/textures/2k_mars.jpg",
            GL_LINEAR
        );
        texture_snowball = std::make_unique<Texture>(
            "assets/textures/snowball.png",
            GL_LINEAR
        );
        logger::info("Texture loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load texture: {0}", e.what());
        return false;
    }

    // Load vehicle texture
    try
    {
        vehicleTexture = std::make_unique<Texture>(
            "assets/textures/carbon_fiber.jpg",
            GL_LINEAR
        );
        logger::info("Vehicle texture loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load vehicle texture: {0}", e.what());
        return false;
    }

    // Load skybox texture
    try
    {
        skyboxTexture = std::make_unique<Texture>(
            "assets/textures/sky/snow_landscape.hdr",
            GL_LINEAR
        );
        logger::info("Skybox texture loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::warn("Failed to load skybox texture: {0}", e.what());
        return false;
    }

    // Create geometry using GPU_Geometry (RAII)
    triangleGeometry = std::make_unique<GPU_Geometry>();
    triangleCPUData = std::make_unique<CPU_Geometry>();
    
    // Generate square using ShapeGenerator (better for textures)
    *triangleCPUData = ShapeGenerator::sphere(1, 16, 16);

    // Upload to GPU
    triangleGeometry->Update(*triangleCPUData);
    
    logger::info("Geometry initialized");

    // Create cube geometry for physics objects
    cubeGeometry = std::make_unique<GPU_Geometry>();
    cubeCPUData = std::make_unique<CPU_Geometry>();
    
    // Generate cube using ShapeGenerator
    *cubeCPUData = ShapeGenerator::unit_cube();
    
    // Upload cube to GPU
    cubeGeometry->Update(*cubeCPUData);
    
    logger::info("Cube geometry initialized");

    // Create skybox geometry (large sphere with more detail)
    skyboxGeometry = std::make_unique<GPU_Geometry>();
    skyboxCPUData = std::make_unique<CPU_Geometry>();
    *skyboxCPUData = ShapeGenerator::sphere(100.0f, 32, 32);
    skyboxGeometry->Update(*skyboxCPUData);
    logger::info("Skybox geometry initialized");

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

Renderable RenderingSystem::getCubeRenderable()
{
    return Renderable{
        .geometry = cubeGeometry.get(),
        .cpuData = cubeCPUData.get(),
        .shader = shader.get(),
        .texture = vehicleTexture.get()
    };
}

// TODO: refactor to avoid code duplication with createSphereEntity
Entity RenderingSystem::createSkyboxEntity()
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
            .geometry = skyboxGeometry.get(),
            .cpuData = skyboxCPUData.get(),
            .shader = shader.get(),
            .texture = skyboxTexture.get(),
            .isSkybox = true
        }
    );

    logger::info("Skybox entity created");

    return skybox;
}

//Create Entity and add PhysxTransform and Renderable Components
Entity RenderingSystem::createSphereEntity()
{
    // Create sphere entity with earth texture as a basis
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
            triangleGeometry.get(),
            triangleCPUData.get(),
            shader.get(),
            texture.get()
        }
    );

    logger::info("Sphere initialized");

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
            ModelRenderable{modelLoader, modelShader.get()}
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
    // Use shader
    shader->use();
    
    // Bind vehicle texture to texture unit 0
    glActiveTexture(GL_TEXTURE0);
    vehicleTexture->bind();
    glUniform1i(glGetUniformLocation(*shader, "baseColorTexture"), 0);
    
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
        
        // Draw the cube
        if (!cubeCPUData->indices.empty()) {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cubeCPUData->indices.size()), GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cubeCPUData->positions.size()));
        }
    }
}

glm::mat4 RenderingSystem::getProjectionMatrix() const
{
    float const aspectRatio = static_cast<float>(window->getWidth()) / static_cast<float>(window->getHeight());
    
    // Perspective projection for active camera
    // FOV is already in radians, no conversion needed
    return glm::perspective(activeCamera->getFOV(), aspectRatio, 0.1f, 300.0f);
}

void RenderingSystem::update(float deltaTime)
{
    processInput(deltaTime);
    
    // Get settings from panel
    glm::vec3 bgColor = imguiPanel->getBackgroundColor();
    glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
    
    // Set viewport
    glViewport(0, 0, window->getWidth(), window->getHeight());
    
    // Apply wireframe mode if enabled
    if (imguiPanel->showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // Update camera stats for UI
    imguiPanel->cameraStats = activeCamera->getStats();
    imguiPanel->cameraStats.aspect = static_cast<float>(window->getWidth()) / static_cast<float>(window->getHeight());
    
    // Render the rotating sphere (demo)
    render();
}

void RenderingSystem::updateUI()
{
    // Begin ImGui frame
    imguiWrapper->beginFrame();

    imguiWrapper->renderFPS();
    imguiPanel->cameraStats = activeCamera->getStats();
    imguiPanel->update();

    // Finish ImGui frame
    imguiWrapper->endFrame();
}

void RenderingSystem::endFrame()
{
    // Swap buffers and poll events
    window->swapBuffers();
    glfwPollEvents();
}

bool RenderingSystem::shouldClose() const
{
    return window->shouldClose();
}

void RenderingSystem::onResize(int width, int height)
{
    glViewport(0, 0, width, height);
    logger::info("Window resized to {}x{}", width, height);
}

void RenderingSystem::onMouseWheelChange(double xOffset, double yOffset)
{
    float scroll = -static_cast<float>(yOffset) * imguiPanel->camZoomSpeed * 0.016f;
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

void RenderingSystem::cleanup() {
    if (imguiWrapper) {
        imguiWrapper->shutdown();
        imguiWrapper.reset();
    }
}
