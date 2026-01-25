#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class ImGuiWrapper {
public:
    ImGuiWrapper() = default;

    bool init(GLFWwindow* window);
    void shutdown();
    void beginFrame();
    void endFrame();

    void renderFPS();

private:
    bool initialized = false;
};
