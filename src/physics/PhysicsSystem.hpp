#pragma once

#include "PxPhysicsAPI.h"
#include "VehicleFourWheelDrive.hpp"
#include "common/PhysicsCallback.hpp"
#include "components/Entity.h"
#include "components/Transform.h"
#include <vector>
#include <iostream>

#include "ecs/Coordinator.hpp"
#include "components/Renderable.h"
#include "components/Transform.h"
#include "components/RigidBody.h"
#include "components/VehicleComponent.h"
#include "core/RenderingSystem.hpp"

#include "core/render/ShaderProgram.hpp"
#include "core/buffer/Geometry.hpp"

extern Coordinator gCoordinator;

class PhysicsSystem : public System {
public:
    std::vector<EntityPx> entityList;

    PhysicsSystem();
    ~PhysicsSystem();

    void update(float deltaTime);
    physx::PxVec3 getPos(int i);

    std::vector<PhysxTransform*> transformList;
    //void updateTransforms();

    Entity createVehicleEntity();
    void spawnBoxPyramid(physx::PxU32 size, float halfLen, Renderable cubeRenderable);

    RigidBody createRigidBodyFromSphere(Entity entity, float radius = 1.f);

    RigidBody createRigidBodyFromMesh(Entity entity);

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

    ContactReportCallback* mContactReportCallback = NULL;

    // Box position tracker
    physx::PxVec3 lastBoxPos;
};
