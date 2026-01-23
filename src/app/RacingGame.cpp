
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
        renderer.loop();
    }
}
