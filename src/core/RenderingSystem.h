#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class RenderingSystem {
public:
    RenderingSystem() = default;
    ~RenderingSystem() = default;

    bool init();
    void loop();
    void cleanup();
    bool shouldClose() const;

private:
    GLFWwindow* window = nullptr;
    
    // OpenGL objects
    unsigned int shaderProgram = 0;
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    
    void processInput();
    bool initShaders();
    bool initGeometry();
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};
