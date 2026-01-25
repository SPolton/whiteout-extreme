#pragma once

#include "PxPhysicsAPI.h"

class PhysicsTest {
public:
    PhysicsTest();

    void initBoxTest();
    void loop();

private:
    //PhysX management class instances.
    physx::PxDefaultAllocator gAllocator;
    physx::PxDefaultErrorCallback gErrorCallback;
    physx::PxFoundation* gFoundation = NULL;
    physx::PxPhysics* gPhysics = NULL;
    physx::PxDefaultCpuDispatcher* gDispatcher = NULL;
    physx::PxScene* gScene = NULL;
    physx::PxMaterial* gMaterial = NULL;
    physx::PxPvd* gPvd = NULL;

    int initPhysicsTest();
};
