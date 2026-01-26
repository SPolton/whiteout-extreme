#pragma once

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
    
    // OpenGL objects
    std::unique_ptr<Shader> shader;
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    
    // ImGui wrapper
    std::unique_ptr<ImGuiWrapper> imguiWrapper;

    bool init();
    void processInput();
    bool initShaders();
    bool initGeometry();
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};
