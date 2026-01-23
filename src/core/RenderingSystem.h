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
    void processInput();
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};
