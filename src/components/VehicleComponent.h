#pragma once

#include "physics/VehicleFourWheelDrive.hpp"
#include <memory>
#include <random>
#include "ecs/Types.hpp"


/**
 * @brief ECS handle for a vehicle plus lightweight gameplay state.
 */
struct VehicleComponent {
    int playerID = 0;  // Optional: Unique identifier for the vehicle

    std::shared_ptr<VehicleFourWheelDrive> instance;

    // Visual smoothing states (for rendering purposes, not used in physics)
    float visualSteer = 0.0f;
    float smoothedVisualSteer = 0.0f;

    //BOOST SYSTEM
    float boostTimer = 4.0f;
    bool isBoosting = false;

    float engineHeat = 0.0f;       // 0.0f to 100.0f (or 0.0 to 1.0)
    float timeSinceLastBoost = 0.0f;
    float maxHeat = 1.0f;
    float heatCooldownTimer = 0.0f; // for 1s delay befor cooldown
    bool isOverheated = false;      // optional : to block boost at 100%
    bool engineFreezing = false;
    bool inSnowStream = false;
    float timeInSnowStream = 0.0f;
    bool boostMaster = false;
    float timeSinceBoostMaster = 0.0f;
    float boostMasterBonus = 0.0f;
    float boostMasterAccuracy = 0.0f;

    // Tuning settings
    float boostHeatPerSecond = 0.18f;  // then linear heating (18% per sec)
    float heatRecoveryDelay = 1.0f;

    float boostHeatInstantCost() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> dis(0.05f, 0.15f);

        return dis(gen);
    }
    // Snowball cooldown
    float snowBallCooldown = 0.0f;

    // Vehicle flip timer
    float flipTimer = 0.0f;

    Entity chassisVisual = 0;
    Entity handleVisual = 0;
    Entity runnerLeftVisual = 0;
    Entity runnerRightVisual = 0;
};
