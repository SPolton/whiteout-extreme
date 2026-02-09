#pragma once

#include "app/GameTime.hpp"
#include "core/RenderingSystem.hpp"
#include "core/Text.h"
#include "physics/PhysicsSystem.hpp"

#include <iostream>
#include <memory>

class RacingGame {
public:
    RacingGame();

    void run();

private:
    GameTime gameTime;
    //std::unique_ptr<RenderingSystem> renderer;
    std::shared_ptr<RenderingSystem> renderingSystem;
    std::unique_ptr<PhysicsSystem> physicsSystem;

    std::unique_ptr<Text> textSystem;
};
