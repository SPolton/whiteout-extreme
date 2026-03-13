#pragma once

#include "PxPhysicsAPI.h"
#include "objects/PhysicsObject.hpp"
#include "vehiclecommon/SnippetVehicleHelpers.h"
#include "vehiclecommon/enginedrivetrain/EngineDrivetrain.h"

using namespace snippetvehicle;

class VehicleFourWheelDrive : public PhysicsObject {
public:
    struct ConstructData {
        //Give the vehicle a name so it can be identified in PVD.
        const char* vehicleName;
        const char* vehicleDataPath;
        const physx::PxVec3 gravity;
        physx::PxPhysics* physics;
        physx::PxScene* scene;
        physx::PxMaterial* material;
    };
    VehicleFourWheelDrive(ConstructData info);
    ~VehicleFourWheelDrive();

    void stepPhysics(float deltaTime);

    // Override from PhysicsObject
    physx::PxRigidActor* getRigidActor() override;

    void setInputs(float throttle, float brake, float steer) {
        mCurrentThrottle = throttle;
        mCurrentBrake = brake;
        mCurrentSteer = steer;
    }

    physx::vehicle2::PxVehicleDirectDriveTransmissionCommandState::Enum setTargetGear(physx::vehicle2::PxVehicleDirectDriveTransmissionCommandState::Enum state) {
        mVehicle.mTransmissionCommandState.targetGear = state;
        return state;
    }

    EngineDriveVehicle& getVehicleData() { return mVehicle; }

private:
    EngineDriveVehicle mVehicle;

    void initMaterialFrictionTable(ConstructData info);

    bool initVehicle(ConstructData info);
    void cleanupVehicles();

    //The path to the vehicle json files to be loaded.
    const char* mVehicleDataPath = NULL;

    //Vehicle simulation needs a simulation context
    //to store global parameters of the simulation such as 
    //gravitational acceleration.
    physx::vehicle2::PxVehiclePhysXSimulationContext mVehicleSimulationContext;

    //The mapping between PxMaterial and friction.
    physx::vehicle2::PxVehiclePhysXMaterialFriction mPhysXMaterialFrictions[16];
    physx::PxU32 mNbPhysXMaterialFrictions = 0;
    physx::PxReal mPhysXDefaultMaterialFriction = 1.0f;

    float mCurrentThrottle = 0.f;
    float mCurrentBrake = 0.f;
    float mCurrentSteer = 0.f;
};
