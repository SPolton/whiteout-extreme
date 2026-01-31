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
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    if (!gFoundation)
    {
        logger::error("PxCreateFoundation failed!");
        throw std::runtime_error("Failed to create PhysX foundation!");
    }

    // PVD
    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    // Physics
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
    if (!gPhysics)
    {
        logger::error("PxCreatePhysics failed!");
        throw std::runtime_error("Failed to create PhysX physics!");
    }

    // Scene
    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    gScene = gPhysics->createScene(sceneDesc);

    // Prep PVD
    PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    // Simulate
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 50), *gMaterial);
    gScene->addActor(*groundPlane);

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
gScene->simulate(delta_time);
gScene->fetchResults(true);

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
    PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfLen, halfLen, halfLen), *gMaterial);
    PxU32 size = 30;
    PxTransform tran(PxVec3(0));

    // Create a pyramid of physics-enabled boxes
    for (PxU32 i = 0; i < size; i++)
    {
        for (PxU32 j = 0; j < size - i; j++)
        {
            PxTransform localTran(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 - 1), 0) * halfLen);
            PxRigidDynamic* body = gPhysics->createRigidDynamic(tran.transform(localTran));

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
            gScene->addActor(*body);
        }
    }

    // Prepare transform list for updates
    updateTransforms();

    // Clean up
    shape->release();

    logger::info("Box test spawned successfully.");
}
