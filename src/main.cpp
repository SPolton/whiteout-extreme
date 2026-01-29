// main.cpp : Defines the entry point for the application.
//

// Force discrete GPU on hybrid graphics systems (NVIDIA/AMD)
#include "utils/prefer_discrete_GPU.h"

#include "app/RacingGame.h"
#include "utils/logger.h"

int main()
{
    logger::info("Starting main");

    try
    {
        RacingGame game;
        game.run();
    }
    catch (const std::runtime_error& e)
    {
        logger::fatal("Runtime error: {}", e.what());
        return -1;
    }
    catch (const std::exception& e)
    {
        logger::fatal("Exception caught: {}", e.what());
        return -1;
    }
    catch (...)
    {
        logger::fatal("Unknown exception caught");
        return -1;
    }

    return 0;
}
