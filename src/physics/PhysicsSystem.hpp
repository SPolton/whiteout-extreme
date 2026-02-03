#pragma once

#include "PxPhysicsAPI.h"
#include "components/Entity.h"
#include "components/Transform.h"
#include <vector>
#include <iostream>

class PhysicsSystem {
public:
    std::vector<Entity> entityList;

    // Constructor
    PhysicsSystem();

    void update(float delta_time);
    physx::PxVec3 getPos(int i);

    std::vector<PhysxTransform*> transformList;
    void updateTransforms();
private:
    std::vector<physx::PxRigidDynamic*> rigidDynamicList;

    // PhysX management class instances.
    physx::PxDefaultAllocator gAllocator;
    physx::PxDefaultErrorCallback gErrorCallback;
    physx::PxFoundation* gFoundation = NULL;
    physx::PxPhysics* gPhysics = NULL;
    physx::PxDefaultCpuDispatcher* gDispatcher = NULL;
    physx::PxScene* gScene = NULL;
    physx::PxMaterial* gMaterial = NULL;
    physx::PxPvd* gPvd = NULL;

    physx::PxVec3 lastBoxPos; // Box position tracker

    void initPhysicsSystem();
    void initBoxes();
};
