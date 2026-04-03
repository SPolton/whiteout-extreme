#include "PhysicsSystem.hpp"
#include "common/Flags.hpp"
#include "common/PVD.h"
#include "components/Model.h"
#include "utils/logger.h"

// OK in cpp files, not in headers
using namespace physx;

PhysicsSystem::PhysicsSystem(
    std::shared_ptr<InputManager> inputManager,
    std::shared_ptr<AudioEngine> audioManager)
    : inputManager(inputManager),
    audioManager(audioManager)
{
    // Core PhysX Initialization only (Foundation, PVD, Physics, Scene)
    initPhysX();
    initGroundPlane();
}

PhysicsSystem::~PhysicsSystem()
{
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

    // Create callback with RAII
    mContactReportCallback = std::make_unique<ContactReportCallback>(inputManager, audioManager, &mGameTime);
    sceneDesc.simulationEventCallback = mContactReportCallback.get();

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
    // Clean up all vehicles before PhysX Foundation is released
    for (auto const& entity : mEntities) {
        if (gCoordinator.HasComponent<VehicleComponent>(entity)) {
            auto& vehicleComp = gCoordinator.GetComponent<VehicleComponent>(entity);
            if (vehicleComp.instance) {
                // Force destruction of the PhysX vehicle objects now
                vehicleComp.instance.reset();
            }
        }
    }

    vehicle2::PxCloseVehicleExtension();

    cleanupGroundPlane();

    PX_RELEASE(mMaterial);
    PX_RELEASE(mScene);
    mContactReportCallback.reset();
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
    // Create the Ground Plane Entity
    // We treat the ground as an entity so other systems (like Rendering) can interact with it
    Entity ground = gCoordinator.CreateEntity();

    mGroundPlane = PxCreatePlane(*mPhysics, PxPlane(0, 1, 0, 0), *mMaterial);
    for (PxU32 i = 0; i < mGroundPlane->getNbShapes(); i++)
    {
        PxShape* shape = NULL;
        mGroundPlane->getShapes(&shape, 1, i);
        shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
        shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
        shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
    }
    mScene->addActor(*mGroundPlane);
    logger::info("Ground plane created successfully.");

    // Add the RigidBody component (Static actors don't necessarily need a Transform component 
    // unless they move, but they MUST have a RigidBody for the PhysicsSystem signature)
    gCoordinator.AddComponent(ground, RigidBody{ mGroundPlane });
}

void PhysicsSystem::cleanupGroundPlane()
{
    if (mGroundPlane)
    {
        mGroundPlane->release();
        mGroundPlane = NULL;
    }
}

void PhysicsSystem::update(float deltaTime)
{
    // PRE-SIMULATION PHASE: Update physics-related components before stepping the simulation
    for (auto const& entity : mEntities) {
        if (gCoordinator.HasComponent<VehicleComponent>(entity)) {
            auto& vehicle = gCoordinator.GetComponent<VehicleComponent>(entity).instance;
            if (vehicle) {
                vehicle->stepPhysics(deltaTime);
            }
        }
        if (gCoordinator.HasComponent<AvalancheComponent>(entity)) {
            auto& avalanche = gCoordinator.GetComponent<AvalancheComponent>(entity).instance;
            if (avalanche && avalanche->mIsActive) {
                avalanche->stepPhysics(deltaTime);
            }
        }
    }

    // SIMULATION PHASE: Step the PhysX Scene
    mScene->simulate(deltaTime);
    mScene->fetchResults(true);

    // POST-SIMULATION PHASE: Synchronize PhysX back to ECS
    for (auto const& entity : mEntities) {
        auto& rb = gCoordinator.GetComponent<RigidBody>(entity);
        auto& transform = gCoordinator.GetComponent<PhysxTransform>(entity);

        // Sync dynamic and kinematic actors
        physx::PxRigidDynamic* dynamicActor = rb.actor->is<physx::PxRigidDynamic>();

        if (dynamicActor) {
            physx::PxTransform pxPose = dynamicActor->getGlobalPose();
            transform.pos = glm::vec3(pxPose.p.x, pxPose.p.y, pxPose.p.z);
            transform.rot = glm::quat(pxPose.q.w, pxPose.q.x, pxPose.q.y, pxPose.q.z);

            // get the linear velocity and store it in the rigid body for later use
            physx::PxVec3 pxVelocity = dynamicActor->getLinearVelocity();
            rb.linearVelocity = glm::vec3(pxVelocity.x, pxVelocity.y, pxVelocity.z);
        }
    }

    mGameTime += deltaTime; // add to lcoal game time

    // if rumble just started, start counting time
    if (inputManager->isRumbling() && mRumbleTime == 0.0f) {
        mRumbleTime = mGameTime;
    }
    // if rumble has exceeded max duration, stop rumble
    if (inputManager->isRumbling() && mGameTime - mRumbleTime > mRumbleDuration) {
        inputManager->rumble(0.0f); // stop rumble by sending 0
        mRumbleTime = 0.0f;
    }

}

Entity PhysicsSystem::createVehicleEntity(const char* name, physx::PxVec3 spawnPos)
{
    VehicleFourWheelDrive::ConstructData vehicleData{
        .vehicleName = name,
        .vehicleDataPath = "assets/vehicledata",
        .gravity = mGravity,
        .physics = mPhysics,
        .scene = mScene,
        .material = mMaterial
    };

    auto vehicleInstance = std::make_shared<VehicleFourWheelDrive>(vehicleData);

    Entity vehicleEntity = gCoordinator.CreateEntity();

    gCoordinator.AddComponent(vehicleEntity, PhysxTransform{
        glm::vec3(spawnPos.x, spawnPos.y, spawnPos.z),
        glm::quat(1.f, 0.f, 0.f, 0.f),
        glm::vec3(1.65f, 1.4f, 3.75f)
        });

    // RigidBody (using the chassis actor from the vehicle)
    gCoordinator.AddComponent(vehicleEntity, RigidBody{ vehicleInstance->getRigidActor() });

    // Set the actual PhysX actor position to match
    PxTransform pxTransform(spawnPos);
    vehicleInstance->getRigidActor()->setGlobalPose(pxTransform);

    // VehicleComponent (store the vehicle instance for later updates and access)
    gCoordinator.AddComponent(vehicleEntity, VehicleComponent{ .instance = vehicleInstance });

    logger::info("Vehicle entity created with ID: {}", vehicleEntity);

    return vehicleEntity;
}

Entity PhysicsSystem::createAvalancheEntity(const glm::vec3& startPos, float initialSpeed)
{
    // Create the avalanche instance
    Avalanche::ConstructData avalancheData{
        .name = "Avalanche",
        .startPosition = startPos,
        .direction = glm::vec3(0.2f, 0.f, 1.f),
        .initialSpeed = initialSpeed,
        .baseSpeed = 15.0f,
        .maxSpeed = 50.0f,
        //.width = 150.0f,
        //.height = 30.0f,
        //.depth = 40.0f,
        //.width = 28.0f, for flat track demo
        .width = 120.0f,
        .height = 5.0f,
        .depth = 20.0f,
        .physics = mPhysics,
        .scene = mScene,
        .material = mMaterial
    };

    auto avalancheInstance = std::make_shared<Avalanche>(avalancheData);

    // Create a new entity for the avalanche
    Entity avalancheEntity = gCoordinator.CreateEntity();

    // Calculate rotation to match the direction
    glm::vec3 defaultForward(0.f, 0.f, 1.f);
    glm::quat visualRotation = glm::rotation(defaultForward, glm::normalize(avalancheData.direction));

    // Add Transform component
    gCoordinator.AddComponent(avalancheEntity, PhysxTransform{
        startPos,
        visualRotation,
        glm::vec3(avalancheData.width, avalancheData.height, avalancheData.depth)
    });

    gCoordinator.AddComponent(avalancheEntity, RigidBody{ avalancheInstance->mPhysicsActor });
    gCoordinator.AddComponent(avalancheEntity, AvalancheComponent{avalancheInstance});

    logger::info("Avalanche entity created with ID: {}", avalancheEntity);

    return avalancheEntity;
}

void PhysicsSystem::spawnBoxPyramid(physx::PxU32 size, float halfLen, Renderable cubeRenderable)
{
    physx::PxShape* shape = mPhysics->createShape(physx::PxBoxGeometry(halfLen, halfLen, halfLen), *mMaterial);
    physx::PxFilterData boxFilter(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_OBSTACLE_AGAINST, 0, 0);
    shape->setSimulationFilterData(boxFilter);

    for (physx::PxU32 i = 0; i < size; i++) {
        for (physx::PxU32 j = 0; j < size - i; j++) {
            // 1. Calculate Position
            physx::PxVec3 pos(
                physx::PxReal(j * 2) - physx::PxReal(size - i),
                physx::PxReal(i * 2 + 5), // Added +5 to drop them from the air
                0.0f
            );
            pos *= halfLen;

            // 2. Create PhysX Actor
            physx::PxRigidDynamic* body = mPhysics->createRigidDynamic(physx::PxTransform(pos));
            body->attachShape(*shape);
            physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
            mScene->addActor(*body);

            // 3. Create ECS Entity
            Entity box = gCoordinator.CreateEntity();

            // PHYSICS: Add RigidBody and Transform for the PhysicsSystem to track
            gCoordinator.AddComponent(box, RigidBody{ body });
            gCoordinator.AddComponent(box, PhysxTransform{
                glm::vec3(pos.x, pos.y, pos.z),
                glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                glm::vec3(1.f) // Scale
                });

            gCoordinator.AddComponent(box, cubeRenderable);
        }
    }
    shape->release();
}

RigidBody PhysicsSystem::createRigidBodyFromSphere(Entity entity, float radius)
{
    // 1. Retrieve the transform component from the entity
    auto& transform = gCoordinator.GetComponent<PhysxTransform>(entity);

    // 2. Convert the transform to PhysX format
    physx::PxTransform pxTran(
        physx::PxVec3(transform.pos.x, transform.pos.y, transform.pos.z),
        physx::PxQuat(transform.rot.x, transform.rot.y, transform.rot.z, transform.rot.w)
    );

    // 3. Create the rigid dynamic actor in PhysX
    physx::PxRigidDynamic* body = mPhysics->createRigidDynamic(pxTran);

    //Create a simple shape (sphere) and attach it to the body
    physx::PxShape* shape = mPhysics->createShape(physx::PxSphereGeometry(radius), *mMaterial);
    body->attachShape(*shape);

    // Set mass and inertia
    physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);

    // 4. Add the actor to the PhysX scene
    mScene->addActor(*body);

    // Clean up shape as it's now attached to the body
    shape->release();

    logger::info("Physics body created for entity {}", entity);

    // 5. Return the RigidBody component
    return RigidBody{ body };
}

RigidBody PhysicsSystem::createRigidBodyFromMesh(Entity entity)
{
    // Retrieve the transform and model renderable components from the entity
    auto& transform = gCoordinator.GetComponent<PhysxTransform>(entity);
    auto& modelRenderable = gCoordinator.GetComponent<ModelRenderable>(entity);

    // Extract scale (use uniform scale for mesh loading)
    float scale = transform.scale.x;
    const std::string& objPath = modelRenderable.modelLoader->getPath();

    logger::info("Creating collision mesh from already-loaded model: {} ({} meshes)", 
                 objPath, modelRenderable.modelLoader->getMeshCount());

    // Use the already-loaded mesh data from ModelLoader
    const auto& meshes = modelRenderable.modelLoader->getMeshes();

    // Pre-calculate total size to avoid reallocation
    size_t totalVertices = 0;
    size_t totalIndices = 0;
    for (const auto& mesh : meshes) {
        totalVertices += mesh.vertices.size();
        totalIndices += mesh.indices.size();
    }

    // Collect all vertices and indices from all meshes in the loaded model
    std::vector<PxVec3> vertices;
    std::vector<PxU32> indices;
    vertices.reserve(totalVertices);
    indices.reserve(totalIndices);
    
    for (const auto& mesh : meshes) {
        size_t indexOffset = vertices.size();

        // Add vertices with scale applied
        for (const auto& vertex : mesh.vertices) {
            vertices.push_back(PxVec3(
                vertex.Position.x * scale,
                vertex.Position.y * scale,
                vertex.Position.z * scale
            ));
        }

        // Add indices with offset
        for (unsigned int idx : mesh.indices) {
            indices.push_back(static_cast<unsigned int>(indexOffset) + idx);
        }
    }

    logger::info("Collision mesh: {} vertices, {} triangles", vertices.size(), indices.size() / 3);

    // Create PhysX triangle mesh
    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = static_cast<PxU32>(vertices.size());
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = vertices.data();

    meshDesc.triangles.count = static_cast<PxU32>(indices.size() / 3);
    meshDesc.triangles.stride = 3 * sizeof(PxU32);
    meshDesc.triangles.data = indices.data();

    PxDefaultMemoryOutputStream writeBuffer;
    PxTriangleMeshCookingResult::Enum result;
    PxCookingParams params(mPhysics->getTolerancesScale());
    bool status = PxCookTriangleMesh(params, meshDesc, writeBuffer, &result);

    if (!status) {
        logger::error("Failed to cook triangle mesh");
        throw std::runtime_error("Failed to cook triangle mesh");
    }

    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    PxTriangleMesh* triangleMesh = mPhysics->createTriangleMesh(readBuffer);

    // Create static actor with position and rotation from transform
    PxTransform pxTransform(
        PxVec3(transform.pos.x, transform.pos.y, transform.pos.z),
        PxQuat(transform.rot.x, transform.rot.y, transform.rot.z, transform.rot.w)
    );
    PxTriangleMeshGeometry geom(triangleMesh);
    geom.meshFlags = PxMeshGeometryFlag::eDOUBLE_SIDED;
    PxRigidStatic* mapActor = PxCreateStatic(*mPhysics, pxTransform, geom, *mMaterial);

    // Set collision flags
    PxShape* shape = nullptr;
    mapActor->getShapes(&shape, 1);
    if (shape) {
        shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
        shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
    }

    // Add the actor to the scene
    mScene->addActor(*mapActor);

    logger::info("Mesh collision rigid body created successfully for entity {}", entity);

    // Return the RigidBody component
    return RigidBody{ mapActor };
}
