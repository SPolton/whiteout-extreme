#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct GameTime {
    double t = 0.0;
    const double dt = 1.0 / 60.0; // simulate at 60fps
    double currentTime = glfwGetTime();
    double accumulator = 0.0;
    size_t frameCount = 0;
    size_t physicsFrameCount = 0;
    double fps = 0.0;

    // convenience getters for float values
    float dtF() const { return static_cast<float>(dt); }
    float tF() const { return static_cast<float>(t); }
    float fpsF() const { return static_cast<float>(fps); }
    float accF() const { return static_cast<float>(accumulator); }

    // Call at the start of each frame
    void update() {
        // New Time Trackers
        double newTime = glfwGetTime();
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        
        // Clamp frame time to prevent spiral of death
        // Max 0.25 seconds (4 frames at 60fps) prevents teleporting after freezes
        if (frameTime > 0.25) {
            frameTime = 0.25;
        }
        
        accumulator += frameTime;
        frameCount++;

        updateFPS(frameTime);
    }

    // Call after each physics update
    void physicsUpdate() {
        accumulator -= dt;
        t += dt;
        physicsFrameCount++;
    }

    // Calculate max physics steps based on frame time to prevent spiral of death
    size_t maxPhysicsSteps() const {
        // If frame time (fps) is greater than dt, game is running slow
        // Limit physics updates to reduce load
        if (fps > dt * 2.0) {
            // Frame is taking longer than 2 physics steps, limit to 1 step
            return 1;
        }
        else if (fps > dt) {
            // Frame is taking longer than 1 physics step, limit to 2 steps
            return 2;
        }
        // Performance is good, allow up to 8 catch-up steps
        return 8;
    }
    
    // Discard excess accumulator when performance is poor
    void discardExcessTime() {
        if (accumulator > dt * 2.0) {
            accumulator = dt * 2.0;
        }
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
