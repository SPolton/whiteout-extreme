
#include "RacingGame.h"
#include "physics/PhysicsTest.h"

RacingGame::~RacingGame()
{
    if (renderer) {
        renderer->cleanup();
    }
}

void RacingGame::run()
{
    // Initialize smart pointers
    renderer = std::make_unique<RenderingSystem>();
    physicsSystem = std::make_unique<PhysicsSystem>();
    textSystem = std::make_unique<Text>();

    if (!renderer->init())
    {
        std::cout << "Failed to initialize rendering system" << std::endl;
        return;
    }

    while (!renderer->shouldClose())
    {
        gameTime.update();

        // Physics System Loop
        while (gameTime.accumulator >= gameTime.dt) {
            physicsSystem->update(gameTime.dt);
            gameTime.accumulator -= gameTime.dt;
            gameTime.t += gameTime.dt;
        }

        renderer->update();
    }
}
