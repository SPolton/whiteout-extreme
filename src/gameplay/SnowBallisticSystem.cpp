#include "SnowBallisticSystem.hpp"
#include "utils/logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/projection.hpp>

#include <vector>

#include <GLFW/glfw3.h>

extern Coordinator gCoordinator;

extern Entity playerVehicleEntity;


SnowBallisticSystem::SnowBallisticSystem(
    std::shared_ptr<InputManager> inputManager,
    std::shared_ptr<AudioEngine> audioManager,
    std::shared_ptr<RenderingSystem> renderingSystem,
    std::shared_ptr<VehicleControlSystem> vehicleControlSystem)
    : inputManager(inputManager),
    audioManager(audioManager),
    renderingSystem(renderingSystem),
    vehicleControlSystem(vehicleControlSystem)
{
}

/// =========== Update loop ===============

void SnowBallisticSystem::update(float dt) {
    const auto& vehicles = vehicleControlSystem->mEntities;
    std::vector<Entity> entitiesToDestroy;

    for (auto const& entity : mEntities) {
        auto& cTrans = gCoordinator.GetComponent<PhysxTransform>(entity);

        if (gCoordinator.HasComponent<SnowCannon>(entity)) {
            auto& cData = gCoordinator.GetComponent<SnowCannon>(entity);

            // 1. Update Oscillation
            cData.oscillationTimer += dt;

            // Speed: 1.0f (can be adjusted), Range: +/- 20 degrees
            float angle = glm::sin(cData.oscillationTimer * 1.5f) * glm::radians(20.0f);

            // Apply rotation on Y axis relative to initial orientation
            // Note: Assuming you store restRotation or rotate current local Y
            glm::quat extraRot = glm::angleAxis(angle, glm::vec3(0, 1, 0));
            cTrans.rot = cData.restRotation * extraRot; // Apply to base rotation
            glm::vec3 cPos = cTrans.pos;
            glm::vec3 cForward = cTrans.forward();

            for (auto const& vehicle : vehicles) {
                auto& vTrans = gCoordinator.GetComponent<PhysxTransform>(vehicle);
                auto& vComp = gCoordinator.GetComponent<VehicleComponent>(vehicle);
                glm::vec3 relPos = vTrans.pos - cPos;

                // 1. Quick sphere check (40m^2 = 1600)
                float d2 = glm::dot(relPos, relPos);
                if (d2 > 1600.f) continue;

                // 2. Front/Back check (Dot product)
                if (glm::dot(relPos, cForward) > 0) continue;

                // 3. Lateral stream check (10m^2 = 100)
                if (getDistSqToStream(vTrans.pos, cTrans) < 100.f) {
                    vComp.inSnowStream = true;
                }
            }
        }
        else if (gCoordinator.HasComponent<SnowBall>(entity)) {
            auto& bTrans = gCoordinator.GetComponent<PhysxTransform>(entity);
            auto& bData = gCoordinator.GetComponent<SnowBall>(entity);
            bool shouldDestroySnowball = false;

            if (glm::length(bTrans.pos - bData.spawnPos) > bData.maxDistance) {
                shouldDestroySnowball = true;
                //logger::error("snowball reached max distance, will be destroyed");
            }

            for (auto const& vehicle : vehicles) {
                if (bData.launcher == vehicle) continue;

                auto& vTrans = gCoordinator.GetComponent<PhysxTransform>(vehicle);
                auto& vComp = gCoordinator.GetComponent<VehicleComponent>(vehicle);

                // Sphere check for collision
                glm::vec3 relPos = vTrans.pos - bTrans.pos;
                float d2 = glm::dot(relPos, relPos);

                if (d2 <= 12.0f) {
                    shouldDestroySnowball = true;

                    if (gCoordinator.HasComponent<RigidBody>(vehicle)) {
                        auto& rb = gCoordinator.GetComponent<RigidBody>(vehicle);
                        auto* dynamicActor = rb.actor->is<physx::PxRigidDynamic>();

                        if (dynamicActor) {
                            float spinImpulse = 9500.0f;
                            float sideSelect = (rand() % 2 == 0) ? 1.0f : -1.0f;
                            dynamicActor->addTorque(physx::PxVec3(0, spinImpulse * sideSelect, 0), physx::PxForceMode::eIMPULSE);

                            physx::PxTransform pose = dynamicActor->getGlobalPose();
                            physx::PxVec3 backwardDir = -pose.q.rotate(physx::PxVec3(0, 0, 1));

                            float hitForce = 9000.0f;
                            physx::PxVec3 frontHitImpulse = (backwardDir * hitForce) + physx::PxVec3(0, 1000.0f, 0);

                            dynamicActor->addForce(frontHitImpulse, physx::PxForceMode::eIMPULSE);

                        }
                    }

                    // play sound of getting hit by snowball (play at position of snowball)
                    audioManager->playSounds("assets/audio/snowball-hit.wav", { bTrans.pos.x, bTrans.pos.y, bTrans.pos.z }, -8.0f);

                    // if player got hit
                    if (vComp.playerID == 0) {
                        // send vibration feedback for getting hit
                        inputManager->rumble(1.0f);
                    }

                    //logger::error("RACER HIT BY SNOWBALL!");

                    break;
                }
            }

            if (shouldDestroySnowball) {
                entitiesToDestroy.push_back(entity);
            }
        }
    }

    for (Entity entity : entitiesToDestroy) {
        if (gCoordinator.HasComponent<SnowBall>(entity)) {
            gCoordinator.DestroyEntity(entity);
        }
    }
}

float SnowBallisticSystem::getDistSqToStream(const glm::vec3& p, const PhysxTransform& trans) {
    glm::vec3 dir = trans.forward();
    glm::vec3 v = p - trans.pos;
    float projection = glm::dot(v, dir);
    // Projection distance along the stream axis
    glm::vec3 closestPoint = trans.pos + (dir * projection);
    glm::vec3 distVec = p - closestPoint;
    return glm::dot(distVec, distVec);
}

void SnowBallisticSystem::throwSnowball(Entity throwerEntity)
{
    auto& vehicleComponent = gCoordinator.GetComponent<VehicleComponent>(throwerEntity);
    if (vehicleComponent.snowBallCooldown > 0.f) return;

    auto& vehicleTransform = gCoordinator.GetComponent<PhysxTransform>(throwerEntity);
    /* Prints position and orientation of vehicle
    std::cout << "{";
    std::cout << "{" << vehicleTransform.pos.x << "f, "
        << vehicleTransform.pos.y << "f, "
        << vehicleTransform.pos.z << "f},"  << std::endl;
    std::cout << "{" << vehicleTransform.rot.w << "f, "
        << vehicleTransform.rot.x << "f, "
        << vehicleTransform.rot.y << "f, "
        << vehicleTransform.rot.z << "f}";
    std::cout << "}," << std::endl;
    */

    // 1. Safety Check: Ensure the player entity is valid
    if (!gCoordinator.HasComponent<VehicleComponent>(throwerEntity)) return;

    //  ...and that the snow ball cool down is finished
    // auto& vehicleComponent = gCoordinator.GetComponent<VehicleComponent>(playerVehicleEntity);
    if (vehicleComponent.snowBallCooldown > 0.f) return;

    // play sound of throwing snowball (play sound at vehicle that is throwing)
    audioManager->playSounds("assets/audio/snowball-throw.mp3", { vehicleTransform.pos.x, vehicleTransform.pos.y, vehicleTransform.pos.z }, 8.0f);
    //logger::info("Throwing snowball...");

    // Calculate the Forward direction based on the vehicle's current rotation
    // In many coordinate systems, (0, 0, 1) is the local forward axis
    glm::vec3 forward = vehicleTransform.forward(); //renderingSystem->getCameraForward();
    glm::vec3 up = vehicleTransform.up();


    float tiltAmount = 0.06f; // 0.05 - 0.3
    glm::vec3 tiltedForward = forward - (up * tiltAmount);
    glm::vec3 forwardTilted = glm::normalize(tiltedForward);


    // Calculate Spawn Position: 
    // Offset the snowball so it doesn't spawn inside the car's collision box
    glm::vec3 spawnPos = vehicleTransform.pos + (forward * 4.0f) + glm::vec3(0, 1.0f, 0);

    // 3. Create Visual Entity
    Entity snowball = renderingSystem->createSphereEntity("assets/textures/snowball.png");

    auto& ballTrans = gCoordinator.GetComponent<PhysxTransform>(snowball);
    ballTrans.pos = spawnPos;
    ballTrans.rot = vehicleTransform.rot; // Align snowball orientation with the car

    // 4. Setup Physics
    float snowballRadius = 0.7f;
    ballTrans.scale = glm::vec3(snowballRadius);

    // Create the RigidBody and add it to the ECS
    gCoordinator.AddComponent(snowball, gCoordinator.GetSystem<PhysicsSystem>()->createRigidBodyFromSphere(snowball, snowballRadius));

    // 5. Apply Initial Velocity
    physx::PxRigidDynamic* dynamicActor = gCoordinator.GetComponent<RigidBody>(snowball).actor->is<physx::PxRigidDynamic>();
    if (dynamicActor) {
        float launchSpeed = 140.f; // Meters per second
        glm::vec3 velocity = forwardTilted * launchSpeed;

        dynamicActor->setLinearDamping(0.41f);

        // Enable CCD (Continuous Collision Detection)
        dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
        //dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);

        // Pass the velocity vector to PhysX
        dynamicActor->setLinearVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
        dynamicActor->setMass(dynamicActor->getMass() * 0.001);
    }

    gCoordinator.AddComponent(snowball,
        SnowBall{
            .launcher = throwerEntity,
            .spawnPos = ballTrans.pos
        });

    SnowEmitter snowballPreset = {
        .enabled = true,
        .preset = SnowEmitterPreset::SnowBall,
        .spawnRate = 200.0f,
        .particleLifetimeSec = 0.275f,
        .particleSize = 1.8f,
        .color = glm::vec3(0.8f, 0.95f, 1.0f),
    };
    gCoordinator.AddComponent(snowball, snowballPreset);

    vehicleComponent.snowBallCooldown = 4.0f;
}

/// =========== SETUP Functions ===============

void SnowBallisticSystem::init() {
    setupSnowCannonsFromPosAndRot();
}

void SnowBallisticSystem::setupSnowCannonsFromPosAndRot() {
    logger::info("SnowBallisticSystem: Starting batch setup of cannons...");

    for (const auto& cannonData : snowCannonsInitialPosAndRot)
    {
        setupSnowCannonEntity(cannonData.first, cannonData.second);
    }

    logger::info("SnowBallisticSystem: {} cannons initialized.", snowCannonsInitialPosAndRot.size());
}

void SnowBallisticSystem::setupSnowCannonEntity(glm::vec3 position, glm::quat rotation) {
    //PhysxTransform Component
    Entity snowCannonEntity = renderingSystem->createModelEntity("assets/obj/cannon/cannon.obj");
    auto& snowCannonTrans = gCoordinator.GetComponent<PhysxTransform>(snowCannonEntity);

    //Scale
    snowCannonTrans.scale = glm::vec3(4.0f);

    //Position
    glm::vec3 offset = glm::vec3(0.f, 2.67f, 0.f);
    snowCannonTrans.pos = position + offset;

    //Rotation
    //glm::quat extraRotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 1, 0));
    snowCannonTrans.rot = rotation;

    //SnowCannon Component
    SnowCannon cannonComp;
    cannonComp.restRotation = snowCannonTrans.rot;
    gCoordinator.AddComponent(snowCannonEntity, cannonComp);

    SnowEmitter emitter;
    emitter.enabled = true;
    emitter.preset = SnowEmitterPreset::SnowCannon;
    emitter.spawnRate = 50.f;
    emitter.particleLifetimeSec = 1.0f;
    emitter.particleSize = 2.5f;
    emitter.color = {0.92f, 0.96f, 1.0f};
    gCoordinator.AddComponent(snowCannonEntity, emitter);

    SnowEmitterGridBox grid;
    grid.enabled = true;
    grid.localBoxSize = glm::vec3(2.5f, 2.5f, 0.1f);
    grid.gridResolution = glm::ivec3(2, 2, 1);
    grid.localOffset = glm::vec3(0.0f, 0.0f, 1.5f);
    grid.pattern = SnowEmitterGridPattern::All;
    gCoordinator.AddComponent(snowCannonEntity, grid);
}
