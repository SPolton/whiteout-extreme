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

    // poll controller state first
    inputManager->pollControllerInputs();

    // based on whether controller is connected, choose the correct input system
    if (inputManager->IsControllerConnected()) {
        processControllerInput();
    }
    else {
        processKeyboardInput();
    }
}

// Input Systems (Keyboard or Controller)
//==================================================================================================================//

void RenderingSystem::processControllerInput() {

}
    
void RenderingSystem::processKeyboardInput() {
    /*
    * WASD (or arrow keys) to move/steer
    * W (up arrow) = accelerate
    * S (down arrow) = brake
    * A (left arrow) = steer left
    * D (right arrow) = steer right
    */

    if (inputManager->IsKeyboardButtonDown(GLFW_KEY_W) || inputManager->IsKeyboardButtonDown(GLFW_KEY_UP)) {
        accelerate();
    }
    else if (inputManager->IsKeyboardButtonDown(GLFW_KEY_S) || inputManager->IsKeyboardButtonDown(GLFW_KEY_DOWN)) {
        brake();
    }

    // Should be able to steer left or right while accelerating
    if (inputManager->IsKeyboardButtonDown(GLFW_KEY_D) || inputManager->IsKeyboardButtonDown(GLFW_KEY_RIGHT)) {
        steerRight();
    }
    else if (inputManager->IsKeyboardButtonDown(GLFW_KEY_A) || inputManager->IsKeyboardButtonDown(GLFW_KEY_LEFT)) {
        steerLeft();
    }

    /*
    * Key Mappings for additional game functions
    * Left Shift = Nitro
    * Space Bar = Throw Snowball (assuming auto aim for now...otherwise mouse input needed)
    */

    // let us just assume we use one skill at a time
    if (inputManager->IsKeyboardButtonDown(GLFW_KEY_LEFT_SHIFT)) {
        boost();
    }
    else if (inputManager->IsKeyboardButtonDown(GLFW_KEY_SPACE)) {
        throwSnowball();
    }
}

// Input -> Movement
//==================================================================================================================//

void RenderingSystem::accelerate()
{
    logger::info("Pressed W (UP). Accelerating...");
    // apply transformation here to move car forward
}

void RenderingSystem::brake()
{
    logger::info("Pressed S (DOWN). Braking...");
    // apply transformation here to slow car down
}

void RenderingSystem::steerRight()
{
    logger::info("Pressed D (RIGHT). Steer Right...");
    // apply transformation here to steer the car to the right
}

void RenderingSystem::steerLeft()
{
    logger::info("Pressed A (LEFT). Steer Left...");
    // apply transformation here steer the car to the left
}

// Input -> Activate Skills
//==================================================================================================================//

void RenderingSystem::boost()
{
    logger::info("Pressed Left Shift. Activate Boost...");
    // apply transformation here accelerate car even faster due to boost.
    // probably need a CD for this?
}

void RenderingSystem::throwSnowball()
{
    logger::info("Pressed Space Bar. Throw Snowball...");
    // logic to throw snowball here.
    // Will need a CD for this...and how many snowballs can we stack again?
}

//==================================================================================================================//

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

    // Load texture
    try
    {
        texture = std::make_unique<Texture>(
            "assets/textures/2k_earth_daymap.jpg",
            GL_LINEAR
        );
        logger::info("Texture loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load texture: {0}", e.what());
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
    
    // Bind texture to texture unit 0
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    glUniform1i(glGetUniformLocation(*shader, "baseColorTexture"), 0);
    
    // Get projection matrix
    glm::mat4 projection = getProjectionMatrix();
    glUniformMatrix4fv(glGetUniformLocation(*shader, "projection"), 1, GL_FALSE, &projection[0][0]);
    
    // Get view matrix from camera
    glm::mat4 view = camera->getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(*shader, "view"), 1, GL_FALSE, &view[0][0]);
    
    // Simple identity model matrix
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, &model[0][0]);
    
    // Bind and render geometry
    triangleGeometry->bind();
    
    // Check if using indexed rendering (sphere/indexed shapes) or array rendering (triangle/square)
    if (!triangleCPUData->indices.empty()) {
        // Indexed rendering for sphere and other indexed geometry
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(triangleCPUData->indices.size()), GL_UNSIGNED_INT, nullptr);
    } else {
        // Array rendering for simple shapes
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(triangleCPUData->positions.size()));
    }
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
