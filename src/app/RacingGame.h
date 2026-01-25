#pragma once

#include "app/GameTime.h"
#include "core/RenderingSystem.h"
#include "core/Text.h"
#include "physics/PhysicsSystem.h"

#include <iostream>
#include <memory>

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
