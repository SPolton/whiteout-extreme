#pragma once

#include "app/GameTime.hpp"
#include "core/RenderingSystem.hpp"
#include "core/Text.h"
#include "physics/PhysicsSystem.hpp"

#include <iostream>
#include <memory>

class RacingGame {
public:
    RacingGame() = default;

    void run();

private:
    GameTime gameTime;
    std::unique_ptr<RenderingSystem> renderer;
    std::unique_ptr<PhysicsSystem> physicsSystem;

    std::unique_ptr<Text> textSystem;
};
