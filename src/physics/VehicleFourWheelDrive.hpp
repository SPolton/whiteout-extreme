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
    physx::PxDefaultAllocator		gAllocator;
    physx::PxDefaultErrorCallback	gErrorCallback;
    physx::PxFoundation* gFoundation = NULL;
    physx::PxPhysics* gPhysics = NULL;
    physx::PxDefaultCpuDispatcher* gDispatcher = NULL;
    physx::PxScene* gScene = NULL;
    physx::PxMaterial* gMaterial = NULL;
    physx::PxPvd* gPvd = NULL;

    //The path to the vehicle json files to be loaded.
    const char* gVehicleDataPath = NULL;

    //Vehicle simulation needs a simulation context
    //to store global parameters of the simulation such as 
    //gravitational acceleration.
    physx::vehicle2::PxVehiclePhysXSimulationContext gVehicleSimulationContext;

    //Gravitational acceleration
    const physx::PxVec3 gGravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

    //The mapping between PxMaterial and friction.
    physx::vehicle2::PxVehiclePhysXMaterialFriction gPhysXMaterialFrictions[16];
    physx::PxU32 gNbPhysXMaterialFrictions = 0;
    physx::PxReal gPhysXDefaultMaterialFriction = 1.0f;

    //A ground plane to drive on.
    physx::PxRigidStatic* gGroundPlane = NULL;
};
