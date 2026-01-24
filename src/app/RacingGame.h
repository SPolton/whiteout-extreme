#pragma once

#include "core/RenderingSystem.h"
#include "core/Text.h"
#include "physics/PhysicsSystem.h"

#include <iostream>
#include <memory>
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

class RacingGame {
public:
    RacingGame() = default;
    ~RacingGame();

    void run();

private:
    GameTime gameTime;
    std::unique_ptr<RenderingSystem> renderer;
    std::unique_ptr<PhysicsSystem> physicsSystem;

    std::unique_ptr<Text> textSystem;
};
