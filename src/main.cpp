// main.cpp : Defines the entry point for the application.
//

#include <iostream>

// Force discrete GPU on hybrid graphics systems (NVIDIA/AMD)
#include "core/PreferDiscreteGPU.h"

#include "app/RacingGame.h"

int main()
{
    std::cout << "Hello Racing Game" << std::endl;

    RacingGame game;
    game.run();

    return 0;
}
