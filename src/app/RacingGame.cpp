
#include "RacingGame.h"
#include "physics/PhysicsTest.h"

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

    PhysicsTest physicsTest;
    physicsTest.initBoxTest();

    while (!renderer.shouldClose())
    {
        // Temporary PhysX test
        physicsTest.loop();

        renderer.loop();
    }
}
