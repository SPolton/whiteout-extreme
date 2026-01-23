#pragma once

#include <iostream>
#include "core/RenderingSystem.h"

class RacingGame {
public:
    RacingGame() = default;
    ~RacingGame();

    void run();

private:
    RenderingSystem renderer;
};
