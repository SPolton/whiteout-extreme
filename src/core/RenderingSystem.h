#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include <memory>

class RenderingSystem {
public:
    RenderingSystem() = default;
    ~RenderingSystem() = default;

    bool init();
    void update();
    void cleanup();
    bool shouldClose() const;

private:
    GLFWwindow* window = nullptr;
    
    // OpenGL objects
    std::unique_ptr<Shader> shader;
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    
    void processInput();
    bool initShaders();
    bool initGeometry();
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};
