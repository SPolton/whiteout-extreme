#include "RenderingSystem.h"
#include <iostream>
#include <cmath>

void RenderingSystem::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void RenderingSystem::processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

bool RenderingSystem::init()
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "Racing Game", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;

    glViewport(0, 0, 800, 600);

    if (!initShaders())
    {
        std::cout << "Failed to initialize shaders" << std::endl;
        return false;
    }

    if (!initGeometry())
    {
        std::cout << "Failed to initialize geometry" << std::endl;
        return false;
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
        std::cout << "Failed to load shaders: " << e.what() << std::endl;
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

void RenderingSystem::endFrame()
{
    // swap buffers and poll IO events
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void RenderingSystem::cleanup()
{
    if (VAO != 0)
        glDeleteVertexArrays(1, &VAO);
    if (VBO != 0)
        glDeleteBuffers(1, &VBO);
    
    shader.reset();

    glfwTerminate();
}

bool RenderingSystem::shouldClose() const
{
    return window != nullptr && glfwWindowShouldClose(window);
}
