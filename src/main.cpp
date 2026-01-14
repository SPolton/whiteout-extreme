// main.cpp : Defines the entry point for the application.
//

#include "app/RacingGame.h"

int main()
{
    RacingGame game;
    game.test();

    std::cout << "Press Enter to exit...";
    std::cin.get();  // wait for user input

    return 0;
}
