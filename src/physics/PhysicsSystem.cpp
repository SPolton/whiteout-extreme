#include "PhysicsSystem.hpp"
#include "common/PVD.h"
#include "utils/logger.h"

// OK in cpp files, not in headers
using namespace physx;

PhysicsSystem::PhysicsSystem() {
    initPhysX();
    initGroundPlane();

    // Init vehicle system here
    VehicleFourWheelDrive::ConstructData vehicleData{
        .vehicleName = "VehiclePlayer1",
        .vehicleDataPath = "assets/vehicledata",
        .gravity = mGravity,
        .physics = mPhysics,
        .scene = mScene,
        .material = mMaterial
    };
    mVehicleSystem = new VehicleFourWheelDrive(vehicleData);

    // Reserve space for vehicle + boxes in entity list
    entityList.reserve(466);
    
    // Add vehicle entity to the list first
    entityList.push_back(mVehicleSystem->getEntity());
    
    initBoxes();
}

PhysicsSystem::~PhysicsSystem() {
    // Clean up vehicle system first
    if (mVehicleSystem) {
        delete mVehicleSystem;
        mVehicleSystem = nullptr;
    }
    cleanupGroundPlane();
    cleanupPhysX();
}

void PhysicsSystem::initPhysX()
{
    // Initialize PhysX
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);
    if (!mFoundation)
    {
        logger::error("PxCreateFoundation failed!");
        throw std::runtime_error("Failed to create PhysX foundation!");
    }

    // PVD
    mPvd = PxCreatePvd(*mFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
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
    sceneDesc.gravity = mGravity;

    PxU32 numWorkers = 1;
    mDispatcher = PxDefaultCpuDispatcherCreate(numWorkers);
    sceneDesc.cpuDispatcher = mDispatcher;
    sceneDesc.filterShader = snippetvehicle::VehicleFilterShader;

    mScene = mPhysics->createScene(sceneDesc);

    // Prep PVD
    PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }
    mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    // Initialize once for vehicle simulation
    vehicle2::PxInitVehicleExtension(*mFoundation);

    logger::info("PhysX initialized successfully.");
}

void PhysicsSystem::cleanupPhysX()
{
    vehicle2::PxCloseVehicleExtension();

    PX_RELEASE(mMaterial);
    PX_RELEASE(mScene);
    PX_RELEASE(mDispatcher);
    PX_RELEASE(mPhysics);
    if (mPvd)
    {
        PxPvdTransport* transport = mPvd->getTransport();
        PX_RELEASE(mPvd);
        PX_RELEASE(transport);
    }
    PX_RELEASE(mFoundation);

    logger::info("PhysX cleaned up successfully.");
}

void PhysicsSystem::initGroundPlane()
{
    mGroundPlane = PxCreatePlane(*mPhysics, PxPlane(0, 1, 0, 0), *mMaterial);
    for (PxU32 i = 0; i < mGroundPlane->getNbShapes(); i++)
    {
        PxShape* shape = NULL;
        mGroundPlane->getShapes(&shape, 1, i);
        shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
        shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
        shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
    }
    mScene->addActor(*mGroundPlane);
    logger::info("Ground plane created successfully.");
}

void PhysicsSystem::cleanupGroundPlane()
{
    mGroundPlane->release();
    logger::info("Ground plane cleaned up successfully.");
}

PxVec3 PhysicsSystem::getPos(int i)
{
    // get position
    PxVec3 position = rigidDynamicList[i]->getGlobalPose().p;
    return position;
}

void PhysicsSystem::updateTransforms() {
    // Update vehicle transform first (at index 0)
    if (mVehicleSystem) {
        mVehicleSystem->updateTransform();
        // Update the entity list reference
        entityList[0] = mVehicleSystem->getEntity();
    }

    // Update box transforms (starting from index 1)
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

void PhysicsSystem::update(float deltaTime)
{
    mVehicleSystem->stepPhysics(deltaTime);

    mScene->simulate(deltaTime);
    mScene->fetchResults(true);

    updateTransforms();

    // Box at index 51 (50 + 1 for vehicle offset)
    PxVec3 objPos = getPos(50);
    if (objPos.y < lastBoxPos.y) {
        logger::debug("x: {0} y: {1} z: {2}", objPos.x, objPos.y, objPos.z);
        logger::debug("Entity y: {0}", entityList[51].transform->pos.y);
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
