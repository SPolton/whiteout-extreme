#pragma once

#include "PxPhysicsAPI.h"

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
};
