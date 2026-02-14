#include "PhysicsSystem.hpp"
#include "common/Flags.hpp"
#include "common/PVD.h"
#include "utils/logger.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// OK in cpp files, not in headers
using namespace physx;

PhysicsSystem::PhysicsSystem()
{
    // Core PhysX Initialization only (Foundation, PVD, Physics, Scene)
    initPhysX();
}

void PhysicsSystem::init()
{
    // Create the Ground Plane Entity
    // We treat the ground as an entity so other systems (like Rendering) can interact with it
    //Entity ground = gCoordinator.CreateEntity();

    // Create the static actor
    //mGroundPlane = PxCreatePlane(*mPhysics, PxPlane(0, 1, 0, 0), *mMaterial);
    //mScene->addActor(*mGroundPlane);

    // Add the RigidBody component (Static actors don't necessarily need a Transform component 
    // unless they move, but they MUST have a RigidBody for the PhysicsSystem signature)
    //gCoordinator.AddComponent(ground, RigidBody{ mGroundPlane });

    // Create the Player Vehicle Entity
    VehicleFourWheelDrive::ConstructData vehicleData{
        .vehicleName = "VehiclePlayer1",
        .vehicleDataPath = "assets/vehicledata",
        .gravity = mGravity,
        .physics = mPhysics,
        .scene = mScene,
        .material = mMaterial
    };

    // Create the vehicle instance
    mVehicleSystem = new VehicleFourWheelDrive(vehicleData);
}

Entity PhysicsSystem::createVehicleEntity()
{
        // 1. Create a new entity for the vehicle
        Entity vehicleEntity = gCoordinator.CreateEntity();
    
        // 2. Add necessary components to the vehicle entity
        // Transform
        gCoordinator.AddComponent(vehicleEntity, PhysxTransform{
            glm::vec3(50.0f, 6.5f, 14.1f),                // Position
            glm::quat(1.f, 0.f, 0.f, 0.f),           // Identity rotation
            glm::vec3(1.65f, 1.4f, 3.75f)               // Scale
            });
    
        // RigidBody (using the chassis actor from the vehicle)
        gCoordinator.AddComponent(vehicleEntity, RigidBody{ mVehicleSystem->getRigidActor()});

        // Set the actual PhysX actor position to match
        PxTransform pxTransform(PxVec3(50.0f, 6.5f, 14.1f));
        mVehicleSystem->getRigidActor()->setGlobalPose(pxTransform);
    
        // VehicleComponent (store the vehicle instance for later updates and access)
        gCoordinator.AddComponent(vehicleEntity, VehicleComponent{.instance = mVehicleSystem});
    
        logger::info("Vehicle entity created with ID: {}", vehicleEntity);
        
        return vehicleEntity;
}

PhysicsSystem::~PhysicsSystem()
{
    // Clean up vehicle system first
    if (mVehicleSystem) {
        delete mVehicleSystem;
        mVehicleSystem = nullptr;
    }
    //cleanupGroundPlane();
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

    // Create a new Callback
    mContactReportCallback = new ContactReportCallback();
    sceneDesc.simulationEventCallback = mContactReportCallback; // Assign it to our scene

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
        shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
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

void PhysicsSystem::update(float deltaTime)
{
    // PRE-SIMULATION PHASE: Update any vehicle physics and prepare the scene
    for (auto const& entity : mEntities) {
        if (gCoordinator.HasComponent<VehicleComponent>(entity)) {
            auto& vehicle = gCoordinator.GetComponent<VehicleComponent>(entity);
            if (vehicle.instance) {
                vehicle.instance->stepPhysics(deltaTime);
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

        // We only want to sync entities that actually move (Dynamic bodies)
        // Static bodies (like the ground) don't need their transform updated every frame
        physx::PxRigidDynamic* dynamicActor = rb.actor->is<physx::PxRigidDynamic>();

        if (dynamicActor) {
            physx::PxTransform pxPose = dynamicActor->getGlobalPose();

            // Sync Position
            transform.pos = glm::vec3(pxPose.p.x, pxPose.p.y, pxPose.p.z);
            
            // Sync Rotation (PhysX Quat to GLM Quat)
            transform.rot = glm::quat(pxPose.q.w, pxPose.q.x, pxPose.q.y, pxPose.q.z);
        }
    }
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
                -30.75f,                                                    // X 
                physx::PxReal(i * 2) - 4.995f,                              // Y 
                physx::PxReal(j * 2) - physx::PxReal(size - i) + 2.13f    // Z 
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

RigidBody PhysicsSystem::createRigidBodyFromMesh(const std::string& objPath, float scale, const glm::vec3& offset)
{
    // Load the OBJ file using Assimp
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(objPath,
        aiProcess_Triangulate | aiProcess_GenNormals);

    if (!scene || !scene->mRootNode || scene->mNumMeshes == 0) {
        logger::error("Failed to load mesh for collision: {}", objPath);
        throw std::runtime_error("Failed to load mesh for collision");
    }

    logger::info("Loading collision mesh from: {} ({} meshes)", objPath, scene->mNumMeshes);

    // Collect all vertices and indices from all meshes
    std::vector<PxVec3> vertices;
    std::vector<PxU32> indices;

    for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        unsigned int indexOffset = vertices.size();

        // Add vertices with scale and offset
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            vertices.push_back(PxVec3(
                mesh->mVertices[i].x * scale + offset.x,
                mesh->mVertices[i].y * scale + offset.y,
                mesh->mVertices[i].z * scale + offset.z
            ));
        }

        // Add indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            if (face.mNumIndices == 3) {
                indices.push_back(indexOffset + face.mIndices[0]);
                indices.push_back(indexOffset + face.mIndices[1]);
                indices.push_back(indexOffset + face.mIndices[2]);
            }
        }
    }

    logger::info("Collision mesh: {} vertices, {} triangles", vertices.size(), indices.size() / 3);

    // Create PhysX triangle mesh
    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = vertices.size();
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = vertices.data();

    meshDesc.triangles.count = indices.size() / 3;
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

    // Create static actor
    PxTriangleMeshGeometry geom(triangleMesh);
    geom.meshFlags = PxMeshGeometryFlag::eDOUBLE_SIDED;
    PxRigidStatic* mapActor = PxCreateStatic(*mPhysics, PxTransform(PxIdentity), geom, *mMaterial);

    // Set collision flags
    PxShape* shape = nullptr;
    mapActor->getShapes(&shape, 1);
    if (shape) {
        shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
        shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
    }

    // Add the actor to the scene
    mScene->addActor(*mapActor);

    logger::info("Mesh collision rigid body created successfully");

    // Return the RigidBody component
    return RigidBody{ mapActor };
}
