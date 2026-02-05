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
    physx::PxDefaultAllocator mAllocator;
    physx::PxDefaultErrorCallback mErrorCallback;
    physx::PxFoundation* mFoundation = NULL;
    physx::PxPhysics* mPhysics = NULL;
    physx::PxDefaultCpuDispatcher* mDispatcher = NULL;
    physx::PxScene* mScene = NULL;
    physx::PxMaterial* mMaterial = NULL;
    physx::PxPvd* mPvd = NULL;

    physx::PxVec3 lastBoxPos; // Box position tracker

    void initPhysicsSystem();
    void initBoxes();
};
