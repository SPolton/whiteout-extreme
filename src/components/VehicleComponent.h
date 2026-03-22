#pragma once

#include "physics/VehicleFourWheelDrive.hpp"
#include <memory>

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

    bool forwardGearDesired = true;
    PxVehicleDirectDriveTransmissionCommandState::Enum gearState{ PxVehicleDirectDriveTransmissionCommandState::eFORWARD };

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
