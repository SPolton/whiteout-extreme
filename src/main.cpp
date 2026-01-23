// main.cpp : Defines the entry point for the application.
//

#include <iostream>

// Force discrete GPU on hybrid graphics systems (NVIDIA/AMD)
#include "core/PreferDiscreteGPU.h"

#include "app/RacingGame.h"
#include "physics/PhysicsTest.h"

int main()
{
    std::cout << "Hello Racing Game" << std::endl;

    // Temporary PhysX test
    physicsBoxTest();

    RacingGame game;
    game.run();

    return 0;
}
