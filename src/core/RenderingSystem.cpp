#include "RenderingSystem.hpp"
#include "utils/logger.h"
#include "core/render/ShapeGenerator.hpp"

#include <iostream>
#include <cmath>

RenderingSystem::RenderingSystem() {
    if (!init()) {
        throw std::runtime_error("Failed to initialize RenderingSystem!");
    }
}

RenderingSystem::~RenderingSystem()
{
    // ImGui cleanup via wrapper
    if (imguiWrapper) {
        imguiWrapper->shutdown();
    }
}

void RenderingSystem::processInput(float deltaTime)
{
    if (glfwGetKey(window->getGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window->getGLFWwindow(), true);

    // Mouse camera controls
    auto const cursorPosition = inputManager->CursorPosition();

    if (cursorPositionIsSetOnce == true) {
        if (inputManager->IsMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT) == true)
        {
            float const aspectRatio = static_cast<float>(window->getWidth()) / static_cast<float>(window->getHeight());
            auto const deltaPosition = cursorPosition - previousCursorPosition;
            camera->adjustTheta(-static_cast<float>(deltaPosition.x) * deltaTime * imguiPanel->camSpeed * (1 / aspectRatio));
            camera->adjustPhi(-static_cast<float>(deltaPosition.y) * deltaTime * imguiPanel->camSpeed);
        }
    }

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
    window = std::make_unique<Window>(800, 600, "Racing Game");
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
            "assets/shaders/shader.vert", 
            "assets/shaders/shader.frag"
        );
        logger::info("Shaders loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load shaders: {0}", e.what());
        return false;
    }

    // Create geometry using GPU_Geometry (RAII)
    triangleGeometry = std::make_unique<GPU_Geometry>();
    triangleCPUData = std::make_unique<CPU_Geometry>();
    
    // Generate triangle using ShapeGenerator
    *triangleCPUData = ShapeGenerator::Triangle();

    // Upload to GPU
    triangleGeometry->Update(*triangleCPUData);
    
    logger::info("Geometry initialized");

    // Create camera
    camera = std::make_unique<TurnTableCamera>();
    
    logger::info("Camera initialized");

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    return true;
}

void RenderingSystem::render()
{
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use shader
    shader->use();
    
    // Set the uniform color (animated over time)
    float timeValue = static_cast<float>(glfwGetTime());
    float greenValue = (std::sin(timeValue) / 2.0f) + 0.5f;
    glUniform4f(glGetUniformLocation(*shader, "ourColor"), 0.0f, greenValue, 0.0f, 1.0f);
    
    // Get projection matrix
    glm::mat4 projection = getProjectionMatrix();
    glUniformMatrix4fv(glGetUniformLocation(*shader, "projection"), 1, GL_FALSE, &projection[0][0]);
    
    // Get view matrix from camera
    glm::mat4 view = camera->getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(*shader, "view"), 1, GL_FALSE, &view[0][0]);
    
    // Simple identity model matrix
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, &model[0][0]);
    
    // Bind and render triangle
    triangleGeometry->bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

glm::mat4 RenderingSystem::getProjectionMatrix() const
{
    float const aspectRatio = static_cast<float>(window->getWidth()) / static_cast<float>(window->getHeight());
    
    // Perspective projection for TurnTableCamera
    return glm::perspective(camera->getFOV(), aspectRatio, 0.1f, 100.0f);
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
    imguiPanel->cameraStats = camera->getStats();
    imguiPanel->cameraStats.aspect = static_cast<float>(window->getWidth()) / static_cast<float>(window->getHeight());
    
    // Render the scene
    render();
}

void RenderingSystem::updateUI()
{
    // Begin ImGui frame
    imguiWrapper->beginFrame();

    imguiWrapper->renderFPS();
    imguiPanel->cameraStats = camera->getStats();
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
    camera->adjustRadius(scroll);
}
