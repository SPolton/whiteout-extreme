#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct GameTime {
    double t = 0.0;
    const double dt = 1.0 / 60.0; // simulate at 60fps
    double currentTime = glfwGetTime();
    double accumulator = 0.0;

    void update() {
        // New Time Trackers
        double newTime = glfwGetTime();
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        accumulator += frameTime;
    }
};
