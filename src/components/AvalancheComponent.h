#pragma once

#include <vector>
#include "Entity.h"

struct AvalancheComponent {
    float speed = 0.0f;
    float baseSpeed = 15.0f;
    float maxSpeed = 50.0f;
    float accumulatedDistance = 0.0f;
    bool isActive = true;
    
    // Rubber-banding
    float lastPlayerDistance = 0.0f;
    float timeInLastPosition = 0.0f;
    
    // Engulfment tracking - store indices into player entity array
    std::vector<size_t> engulfedPlayerIndices;
    
    // Gameplay state
    float raceProgressPercentage = 0.0f;
    bool autoEngulfEnabled = false;
};
