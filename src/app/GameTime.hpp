#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct GameTime {
    double t = 0.0;
    const double dt = 1.0 / 60.0; // simulate at 60fps
    double currentTime = glfwGetTime();
    double accumulator = 0.0;
    unsigned int frameCount = 0;
    unsigned int physicsFrameCount = 0;
    double fps = 0.0;

    // convenience getters for float values
    float dtF() const { return static_cast<float>(dt); }
    float tF() const { return static_cast<float>(t); }
    float fpsF() const { return static_cast<float>(fps); }
    float accF() const { return static_cast<float>(accumulator); }

    void update() {
        // New Time Trackers
        double newTime = glfwGetTime();
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        accumulator += frameTime;
        frameCount++;

        updateFPS(frameTime);
    }

    void physicsUpdate() {
        accumulator -= dt;
        t += dt;
        physicsFrameCount++;
    }

private:
    double fpsDelta = 0.0;

    void updateFPS(double frameTime) {
        fpsDelta += dt;
        if (fpsDelta >= 0.2) {
            fps = frameTime;
            fpsDelta = 0.0;
        }
    }
};
