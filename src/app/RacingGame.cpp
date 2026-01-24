
#include "RacingGame.h"

RacingGame::~RacingGame()
{
    renderer.cleanup();
}

void RacingGame::run()
{
    if (!renderer.init())
    {
        std::cout << "Failed to initialize rendering system" << std::endl;
        return;
    }

    while (!renderer.shouldClose())
    {
        gameTime.update();

        // Physics System Loop
        //while (accumulator >= dt) {
        //    physicsSys->updatePhysics(dt);
        //    accumulator -= dt;
        //    t += dt;
        //}

        renderer.update();
    }
}
