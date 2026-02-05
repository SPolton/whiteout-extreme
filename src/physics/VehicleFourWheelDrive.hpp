#pragma once

#include "PxPhysicsAPI.h"
#include "vehiclecommon/SnippetVehicleHelpers.h"

class VehicleFourWheelDrive {
public:
    VehicleFourWheelDrive(const char* vehicleDataPath);
    ~VehicleFourWheelDrive();

    void stepPhysics();

private:
    // Initialization and cleanup functions
    void initPhysX();
    void cleanupPhysX();

    void initGroundPlane();
    void cleanupGroundPlane();

    void initMaterialFrictionTable();

    bool initVehicles();
    void cleanupVehicles();

    bool initPhysics();
    void cleanupPhysics();

    //PhysX management class instances.
    physx::PxDefaultAllocator mAllocator;
    physx::PxDefaultErrorCallback mErrorCallback;
    physx::PxFoundation* mFoundation = NULL;
    physx::PxPhysics* mPhysics = NULL;
    physx::PxDefaultCpuDispatcher* mDispatcher = NULL;
    physx::PxScene* mScene = NULL;
    physx::PxMaterial* mMaterial = NULL;
    physx::PxPvd* mPvd = NULL;

    //The path to the vehicle json files to be loaded.
    const char* mVehicleDataPath = NULL;

    //Vehicle simulation needs a simulation context
    //to store global parameters of the simulation such as 
    //gravitational acceleration.
    physx::vehicle2::PxVehiclePhysXSimulationContext mVehicleSimulationContext;

    //Gravitational acceleration
    const physx::PxVec3 mGravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

    //The mapping between PxMaterial and friction.
    physx::vehicle2::PxVehiclePhysXMaterialFriction mPhysXMaterialFrictions[16];
    physx::PxU32 mNbPhysXMaterialFrictions = 0;
    physx::PxReal mPhysXDefaultMaterialFriction = 1.0f;

    //A ground plane to drive on.
    physx::PxRigidStatic* mGroundPlane = NULL;
};
