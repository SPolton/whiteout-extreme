#pragma once

#include "input/Window.hpp"
#include "core/ImGuiWrapper.h"
#include "core/Shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class RenderingSystem {
public:
    RenderingSystem();

    void update();
    void updateUI();
    void endFrame();
    void cleanup();
    bool shouldClose() const;

private:
    GLFWwindow* window = nullptr;
    std::unique_ptr<Window> window;
    
    // OpenGL objects
    std::unique_ptr<Shader> shader;
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    
    // ImGui wrapper
    std::unique_ptr<ImGuiWrapper> imguiWrapper;

    bool init();
    void processInput();
    bool initShaders();
    glm::mat4 getProjectionMatrix() const;
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};
