#pragma once
#include <memory>
#include "physics/VehicleFourWheelDrive.hpp"

/**
 * @brief Component containing the specific logic for a 4WD vehicle.
 * It holds the instance of the vehicle simulation and the current input states.
 */
struct VehicleComponent {
    int playerID = 0;  // Optional: Unique identifier for the vehicle

    // Unique pointer to manage the vehicle instance lifecycle
    VehicleFourWheelDrive* instance = nullptr;

    // Control states (updated by the Input/Rendering system)
    float throttle = 0.0f;
    float brake = 0.0f;
    float steer = 0.0f;

    // Snowball cooldown
    float snowBallCooldown = 0.0f;
};
