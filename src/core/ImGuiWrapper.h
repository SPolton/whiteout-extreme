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

private:
    bool initialized = false;
};
