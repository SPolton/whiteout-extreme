#include "PhysicsTest.hpp"
#include "utils/logger.h"

// OK in cpp files, not in headers
using namespace physx;

PhysicsTest::PhysicsTest() {
    if (initPhysicsTest() != 0)
    {
        throw std::runtime_error("Failed to initialize physics test!");
    }
}

int PhysicsTest::initPhysicsTest() {
    // Initialize PhysX
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    if (!gFoundation)
    {
        logger::error("PxCreateFoundation failed!");
        return -1;
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
        return -1;
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
    return 0;
}

void PhysicsTest::initBoxTest()
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
            body->attachShape(*shape);
            PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
            gScene->addActor(*body);
        }
    }

    // Clean up
    shape->release();

    logger::info("Box test spawned successfully.");
}

void PhysicsTest::loop() {
    // Simulate at 60fps
    gScene->simulate(1.0f / 60.0f);
    gScene->fetchResults(true);
}
