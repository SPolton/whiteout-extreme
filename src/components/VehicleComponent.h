#pragma once

#include "physics/VehicleFourWheelDrive.hpp"
#include <memory>
#include <random>

using namespace physx::vehicle2;

/**
 * @brief Component containing the specific logic for a 4WD vehicle.
 * It holds the instance of the vehicle simulation and the current input states.
 */
struct VehicleComponent {
    int playerID = 0;  // Optional: Unique identifier for the vehicle

    std::shared_ptr<VehicleFourWheelDrive> instance;

    // Control states (updated by the Input/Rendering system)
    float throttle = 0.0f;
    float brake = 0.0f;
    float steer = 0.0f;

    //BOOST SYSTEM
    float boostTimer = 4.0f;
    bool isBoosting = false;

    float engineHeat = 0.0f;       // 0.0f to 100.0f (or 0.0 to 1.0)
    float timeSinceLastBoost = 0.0f;
    float maxHeat = 1.0f;
    float heatCooldownTimer = 0.0f; // for 1s delay befor cooldown
    bool isOverheated = false;      // optional : to block boost at 100%
    bool engineFreezing = false;
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

    bool forwardGearDesired = true;
    PxVehicleDirectDriveTransmissionCommandState::Enum gearState{ PxVehicleDirectDriveTransmissionCommandState::eREVERSE };

    const bool hasGearDesired() {
        return forwardGearDesired ?
            gearState == PxVehicleDirectDriveTransmissionCommandState::eFORWARD :
            gearState == PxVehicleDirectDriveTransmissionCommandState::eREVERSE;
    }

    void setGearDesired() {
        gearState = this->instance->setTargetGear(this->forwardGearDesired ?
            physx::vehicle2::PxVehicleDirectDriveTransmissionCommandState::eFORWARD :
            physx::vehicle2::PxVehicleDirectDriveTransmissionCommandState::eREVERSE
        );
    }

    float speed() {
        return this->instance->getRigidActor()->is<physx::PxRigidBody>()->getLinearVelocity().magnitude();
    }

    // Snowball cooldown
    float snowBallCooldown = 0.0f;

    // Vehicle flip timer
    float flipTimer = 0.0f;
};
