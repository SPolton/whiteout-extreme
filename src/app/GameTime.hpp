#pragma once

#include <string>

struct GameTime {
    const double dt = 1.0 / 60.0; // simulate at 60fps
    size_t frameCount = 0;
    size_t physicsFrameCount = 0;

    // convenience getters for float values
    float dtF() const { return static_cast<float>(dt); }
    float accF() const { return static_cast<float>(accumulator); }
    float fpsF() const { return static_cast<float>(fps); }
    float frameTimeF() const { return static_cast<float>(lastFrameTime); }
    float gameTimeF() const { return static_cast<float>(gameTime); }
    float totalTimeF() const { return static_cast<float>(totalTime); }

    void reset(double newTime = 0.0) {
        totalTime = newTime;
        gameTime = 0.0;
        accumulator = 0.0;
        frameCount = 0;
        physicsFrameCount = 0;
        fps = 0.0;
        lastFrameTime = 0.0;
    }

    void updatePause(double newTime) {
        // Reset to prevent large delta time on resume
        totalTime = newTime;
        accumulator = 0.0;
        lastFrameTime = 0.0;
    }

    // Call at the start of each frame
    void update(double newTime) {
        double frameTime = newTime - totalTime;
        gameTime += frameTime;
        totalTime = newTime;
        
        // Clamp frame time to prevent spiral of death
        // Max 0.25 seconds (4 frames at 60fps) prevents teleporting after freezes
        if (frameTime > 0.25) {
            frameTime = 0.25;
        }

        lastFrameTime = frameTime;
        accumulator += frameTime;
        frameCount++;

        discardExcessAccumulator();
        updateFPS(frameTime);
    }

    // Call after each physics update
    void physicsUpdate() {
        accumulator -= dt;
        physicsFrameCount++;
    }

    // Calculate max physics steps based on frame time to prevent spiral of death
    size_t maxPhysicsSteps() const {
        double ratio = lastFrameTime / dt;

        if (ratio > 2.5) return 1;
        if (ratio > 1.5) return 2;
        if (ratio > 1.0) return 4;
        return 8;
    }

    std::string toString() const {
        return
            "GameTime(game: " + std::to_string(gameTime) +
            ", total: " + std::to_string(totalTime) +
            ", frameTime: " + std::to_string(lastFrameTime) +
            ", accumulator: " + std::to_string(accumulator) +
            ", fps: " + std::to_string(fps) +
            ", frames: " + std::to_string(frameCount) +
            ", physicsFrames: " + std::to_string(physicsFrameCount) + ")";
    }

private:
    // in seconds
    double gameTime = 0.0;      // total elapsed in-game time
    double totalTime = 0.0;     // total time at the last frame
    double lastFrameTime = 0.0; // duration of the last frame
    double accumulator = 0.0;   // physics time step accumulator
    double fps = 0.0;           // smoothed frames per second

    // Discard excess accumulator when performance is poor
    void discardExcessAccumulator() {
        if (accumulator > dt * 4.0) {
            accumulator = dt * 4.0;
        }
    }

    void updateFPS(double frameTime) {
        double instantFps = 1.0 / frameTime;
        fps = mix(fps, instantFps, 0.1); // smooth it
    }

    double mix(double x, double y, double a) const {
        return x + a * (y - x);
    }
};
