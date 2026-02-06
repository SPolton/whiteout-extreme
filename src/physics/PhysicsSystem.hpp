#pragma once

#include "PxPhysicsAPI.h"
#include "VehicleFourWheelDrive.hpp"
#include "components/Entity.h"
#include "components/Transform.h"
#include <vector>
#include <iostream>

class PhysicsSystem {
public:
    std::vector<Entity> entityList;

    PhysicsSystem();
    ~PhysicsSystem();

    void update(float deltaTime);
    physx::PxVec3 getPos(int i);

    std::vector<PhysxTransform*> transformList;
    void updateTransforms();
private:
    // Initialization and cleanup functions
    void initPhysX();
    void cleanupPhysX();

    void initGroundPlane();
    void cleanupGroundPlane();

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

    //Gravitational acceleration
    const physx::PxVec3 mGravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

    //A ground plane to drive on.
    physx::PxRigidStatic* mGroundPlane = NULL;

    // Vehicle system
    VehicleFourWheelDrive* mVehicleSystem = NULL;

    // Box position tracker
    physx::PxVec3 lastBoxPos;

    void initBoxes();
};
