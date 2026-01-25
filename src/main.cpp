// main.cpp : Defines the entry point for the application.
//

#include <iostream>

// Force discrete GPU on hybrid graphics systems (NVIDIA/AMD)
#include "core/PreferDiscreteGPU.h"

#include "app/RacingGame.h"

int main()
{
    std::cout << "Hello Racing Game" << std::endl;

    try
    {
        RacingGame game;
        game.run();
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "Runtime error: " << e.what() << std::endl;
        return -1;
    }
    catch (const std::exception& e)
    {
        std::cout << "Exception caught: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cout << "Unknown exception caught" << std::endl;
        return -1;
    }

    return 0;
}
