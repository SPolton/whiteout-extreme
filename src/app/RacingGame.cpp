
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
    // Initialize smart pointers for game systems
    renderer = std::make_unique<RenderingSystem>();
    physicsSystem = std::make_unique<PhysicsSystem>();
    textSystem = std::make_unique<Text>();

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
        renderer->updateUI();
        textSystem->update();

        // Must be called last
        renderer->endFrame();
    }
}
