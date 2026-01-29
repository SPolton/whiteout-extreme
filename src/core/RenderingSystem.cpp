#include "RenderingSystem.h"
#include "utils/logger.h"

#include <iostream>
#include <cmath>

RenderingSystem::RenderingSystem()
{
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

    if (VAO != 0)
        glDeleteVertexArrays(1, &VAO);
    if (VBO != 0)
        glDeleteBuffers(1, &VBO);

    shader.reset();

    glfwTerminate();
}

void RenderingSystem::processInput()
{
    if (glfwGetKey(window->getGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window->getGLFWwindow(), true);
}

bool RenderingSystem::init()
{
    if (!glfwInit())
    {
        logger::error("Failed to initialize GLFW");
        return false;
    }

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

    glViewport(0, 0, 800, 600);

    if (!initShaders())
    {
        logger::error("Failed to initialize ImGui");
        return false;
    }

    if (!initGeometry())
    {
        std::cout << "Failed to initialize geometry" << std::endl;
        return false;
    }

    imguiWrapper = std::make_unique<ImGuiWrapper>();
    if (!imguiWrapper->init(window))
    {
        std::cout << "Failed to initialize ImGui" << std::endl;
        //return false;
    }

    return true;
}

bool RenderingSystem::initShaders()
{
    try
    {
        shader = std::make_unique<Shader>("assets/shaders/shader.vert", "assets/shaders/shader.frag");
        std::cout << "Shaders loaded successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load shaders: {0}", e.what());
        return false;
    }
}

bool RenderingSystem::initGeometry()
{
    float vertices[] = {
        // positions only (shader.frag uses uniform color)
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return true;
}

void RenderingSystem::update()
{
    processInput();

    // clear the colorbuffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader->use();

    // Set the uniform color (animated over time)
    float timeValue = static_cast<float>(glfwGetTime());
    float greenValue = (std::sin(timeValue) / 2.0f) + 0.5f;
    shader->setVec4("ourColor", glm::vec4(0.0f, greenValue, 0.0f, 1.0f));

    // now render the triangle
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

glm::mat4 RenderingSystem::getProjectionMatrix() const
{
    float const aspectRatio = static_cast<float>(window->getWidth()) / static_cast<float>(window->getHeight());
}

void RenderingSystem::updateUI()
{
    imguiWrapper->beginFrame();
    imguiWrapper->renderFPS();
    imguiWrapper->endFrame();
}

void RenderingSystem::endFrame()
{
    // swap buffers and poll IO events
    glfwSwapBuffers(window);
    glfwPollEvents();
}

bool RenderingSystem::shouldClose() const
{
    return window != nullptr && glfwWindowShouldClose(window);
}
