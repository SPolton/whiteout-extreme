#include "PhysicsSystem.hpp"
#include "utils/logger.h"

// OK in cpp files, not in headers
using namespace physx;

PhysicsSystem::PhysicsSystem() {
    initPhysicsSystem();

    // Reserve space for boxes in entity list
    entityList.reserve(465);
    initBoxes();
}

void PhysicsSystem::initPhysicsSystem() {
    // Initialize PhysX
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);
    if (!mFoundation)
    {
        logger::error("PxCreateFoundation failed!");
        throw std::runtime_error("Failed to create PhysX foundation!");
    }

    // PVD
    mPvd = PxCreatePvd(*mFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    mPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    // Physics
    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(), true, mPvd);
    if (!mPhysics)
    {
        logger::error("PxCreatePhysics failed!");
        throw std::runtime_error("Failed to create PhysX physics!");
    }

    // Scene
    PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    mDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = mDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    mScene = mPhysics->createScene(sceneDesc);

    // Prep PVD
    PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    // Simulate
    mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    PxRigidStatic* groundPlane = PxCreatePlane(*mPhysics, PxPlane(0, 1, 0, 50), *mMaterial);
    mScene->addActor(*groundPlane);

    logger::info("Physics Test initialized successfully.");
}

PxVec3 PhysicsSystem::getPos(int i)
{
    // get position
    PxVec3 position = rigidDynamicList[i]->getGlobalPose().p;
    return position;
}

void PhysicsSystem::updateTransforms() {

    for (int i = 0; i < transformList.size(); i++) {

        // store positions
        transformList[i]->pos.x = rigidDynamicList[i]->getGlobalPose().p.x;
        transformList[i]->pos.y = rigidDynamicList[i]->getGlobalPose().p.y;
        transformList[i]->pos.z = rigidDynamicList[i]->getGlobalPose().p.z;

        // store rotations
        transformList[i]->rot.x = rigidDynamicList[i]->getGlobalPose().q.x;
        transformList[i]->rot.y = rigidDynamicList[i]->getGlobalPose().q.y;
        transformList[i]->rot.z = rigidDynamicList[i]->getGlobalPose().q.z;
        transformList[i]->rot.w = rigidDynamicList[i]->getGlobalPose().q.w;
    }
}

void PhysicsSystem::update(float delta_time) {
mScene->simulate(delta_time);
mScene->fetchResults(true);

    updateTransforms();

    PxVec3 objPos = getPos(50);
    if (objPos.y < lastBoxPos.y) {
        logger::debug("x: {0} y: {1} z: {2}", objPos.x, objPos.y, objPos.z);
        logger::debug("Entity y: {0}", entityList[50].transform->pos.y);
    }
    lastBoxPos = objPos;
}

void PhysicsSystem::initBoxes()
{
    // Define a box
    float halfLen = 0.5f;
    PxShape* shape = mPhysics->createShape(PxBoxGeometry(halfLen, halfLen, halfLen), *mMaterial);
    PxU32 size = 30;
    PxTransform tran(PxVec3(0));

    // Create a pyramid of physics-enabled boxes
    for (PxU32 i = 0; i < size; i++)
    {
        for (PxU32 j = 0; j < size - i; j++)
        {
            PxTransform localTran(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 - 1), 0) * halfLen);
            PxRigidDynamic* body = mPhysics->createRigidDynamic(tran.transform(localTran));

            // Store rigid body
            rigidDynamicList.push_back(body);

            // Create and store Transform
            PhysxTransform* transform = new PhysxTransform();
            transformList.push_back(transform);

            // Create and store Entity
            Entity entity;
            entity.name = "Box";
            entity.transform = transform;
            entity.model = NULL;
            entityList.push_back(entity);

            body->attachShape(*shape);
            PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
            mScene->addActor(*body);
        }
    }

    // Prepare transform list for updates
    updateTransforms();

    // Clean up
    shape->release();

    logger::info("Box test spawned successfully.");
}
