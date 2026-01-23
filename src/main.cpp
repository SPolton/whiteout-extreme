// main.cpp : Defines the entry point for the application.
//

#include "app/RacingGame.h"

int main()
{
    std::cout << "Hello Racing Game" << std::endl;

    RacingGame game;
    game.run();

    //std::cout << "Press Enter to exit...";
    //std::cin.get();  // wait for user input

    return 0;
}
