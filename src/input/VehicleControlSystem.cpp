#include "VehicleControlSystem.hpp"
#include "utils/logger.h"
//#include "app/RacingGame.cpp"

#include <GLFW/glfw3.h>

extern Coordinator gCoordinator;
extern Entity playerVehicleEntity;

VehicleControlSystem::VehicleControlSystem(
    std::shared_ptr<InputManager> inputManager,
    std::shared_ptr<AudioEngine> audioManager,
    std::shared_ptr<RenderingSystem> renderingSystem,
    std::shared_ptr<PhysicsSystem> physicsSystem)
    : inputManager(inputManager),
    audioManager(audioManager),
    renderingSystem(renderingSystem),
    physicsSystem(physicsSystem)
{
}

void VehicleControlSystem::update(float deltaTime)
{
    if (!inputManager) return;

    resetInputs();
    processInputs();

    for (auto const& entity : mEntities) {
        auto& vehicle = gCoordinator.GetComponent<VehicleComponent>(entity);
        if (!vehicle.instance) {
            continue;
        }
        auto& engineParams = vehicle.instance->getVehicleData().mEngineDriveParams.engineParams;

        // Reset flag isBoosting by default for this frame
        bool wasBoosting = vehicle.isBoosting;

        // Call inputs
        if (entity == playerVehicleEntity) {
            vehicle.isBoosting = false;
            processInputs();
        }

        if (vehicle.isBoosting) {
            // --- LOGIC BOOST ---
            engineParams.maxOmega = 2100.0f;
            vehicle.engineHeat = std::min(1.0f, vehicle.engineHeat + vehicle.boostHeatPerSecond * deltaTime);
            vehicle.timeSinceLastBoost = 0.0f;
        }
        else {
            // --- LOGIC COOLING ---
            engineParams.maxOmega = 1300.0f;
            vehicle.timeSinceLastBoost += deltaTime;

            if (vehicle.timeSinceLastBoost > 1.0f && vehicle.engineHeat > 0.0f) {

                // k increases slightly  : 0.05 at t=1s, 0.20 at t=2s, 0.45 at t=3s...
                float t = vehicle.timeSinceLastBoost - 1.0f;
                float decayRate = 0.05f * (t * t) + 0.02f;

                vehicle.engineHeat -= decayRate * deltaTime;

                if (vehicle.engineHeat < 0.0f) vehicle.engineHeat = 0.0f;
            }
        }

        if (vehicle.snowBallCooldown > 0.f) vehicle.snowBallCooldown -= deltaTime;

        // Check if vehicle is flipped and needs to be reset
        if (gCoordinator.HasComponent<RigidBody>(entity)) {
            auto& rb = gCoordinator.GetComponent<RigidBody>(entity);

            physx::PxTransform pose = rb.actor->getGlobalPose();
            physx::PxVec3 upVector = pose.q.rotate(physx::PxVec3(0, 1, 0));

            bool isGrounded = false;
            if (auto* dynamicActor = rb.actor->is<physx::PxRigidDynamic>()) {
                physx::PxVec3 velocity = dynamicActor->getLinearVelocity();
                isGrounded = (abs(velocity.y) < 2.0f);
            }

            if (upVector.y < 0.6f && isGrounded) {
                vehicle.flipTimer += deltaTime;

                if (vehicle.flipTimer >= 0.5f) {
                    physx::PxQuat uprightRotation = physx::PxQuat(physx::PxIdentity);
                    physx::PxTransform newPose(pose.p + physx::PxVec3(0, 3, 0), uprightRotation);
                    rb.actor->setGlobalPose(newPose);

                    if (auto* dynamicActor = rb.actor->is<physx::PxRigidDynamic>()) {
                        dynamicActor->setLinearVelocity(physx::PxVec3(0, 0, 0));
                        dynamicActor->setAngularVelocity(physx::PxVec3(0, 0, 0));
                    }

                    vehicle.flipTimer = 0.0f;
                }
            }
            else {
                vehicle.flipTimer = 0.0f;
            }
        }

        if(vehicle.playerID == 0) {
            vehicle.throttle = currentThrottle;
            vehicle.brake = currentBrake;
            vehicle.steer = currentSteer;

            if (!vehicle.hasGearDesired()) {
                if (vehicle.speed() < 1.f) {
                    vehicle.setGearDesired();
                }
            }
        }

        vehicle.instance->setInputs(
            vehicle.throttle,
            vehicle.brake,
            vehicle.steer
        );
        //vehicle.instance->stepPhysics(deltaTime);
    }
}

void VehicleControlSystem::processInputs()
{
    // poll controller state first
    inputManager->pollControllerInputs();

    // based on whether controller is connected
    if (inputManager->isControllerConnected())
        processControllerInput();

    processKeyboardInput();
}


void VehicleControlSystem::processControllerInput()
{
    /*
    * Right Trigger = accelerate
    * Left Trigger = brake
    * Left Stick = steering
    * X (switch) = Y (Xbox) = Nitro
    * Y (switch) = X (Xbox) = Throw Snowball (assuming auto aim for now...otherwise right stick input needed?)
    */

    // check for throttle/braking
    // anything greater than 0 means it is pressed
    if (inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.0f) {
        accelerate();
    }
    else if (inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER) > 0.0f) {
        brake();
    }

    // add "deadzone" (since most controllers won't return back to 0.0 exactly)
    float deadZone = 0.2f;

    // check for steering on left joystick
    // if positive, it is steering right
    // if negative, it is steering left
    if (inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_LEFT_X) > deadZone) {
        steerRight();
    }
    else if (inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_LEFT_X) < -deadZone) {
        steerLeft();
    }

    // if top button pressed, activate boost
    // if left button pressed, throw projectile
    if (inputManager->isControllerButtonPressed(GLFW_GAMEPAD_BUTTON_Y)) {
        boost();
    }
    else if (inputManager->isControllerButtonPressed(GLFW_GAMEPAD_BUTTON_X)) {
        throwSnowball();
    }

    // triggers menu/pause
    if (inputManager->isControllerButtonPressed(GLFW_GAMEPAD_BUTTON_START)) {
        //gamePaused();
    }
}

void VehicleControlSystem::processKeyboardInput()
{
    /*
    * WASD (or arrow keys) to move/steer
    * W (up arrow) = accelerate
    * S (down arrow) = brake
    * A (left arrow) = steer left
    * D (right arrow) = steer right
    */

    if (inputManager->isKeyPressed(GLFW_KEY_W) || inputManager->isKeyPressed(GLFW_KEY_UP)) {
        accelerate();
    }
    else if (inputManager->isKeyPressed(GLFW_KEY_S) || inputManager->isKeyPressed(GLFW_KEY_DOWN)) {
        brake();
    }

    // Should be able to steer left or right while accelerating
    if (inputManager->isKeyPressed(GLFW_KEY_D) || inputManager->isKeyPressed(GLFW_KEY_RIGHT)) {
        steerRight();
    }
    else if (inputManager->isKeyPressed(GLFW_KEY_A) || inputManager->isKeyPressed(GLFW_KEY_LEFT)) {
        steerLeft();
    }

    /*
    * Key Mappings for additional game functions
    * Left Shift = Nitro
    * Space Bar = Throw Snowball (assuming auto aim for now...otherwise mouse input needed)
    */

    // let us just assume we use one skill at a time
    if (inputManager->isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        boost();
    }
    else if (inputManager->isKeyPressed(GLFW_KEY_SPACE) || inputManager->isKeyPressed(GLFW_KEY_E)) {
        throwSnowball();
    }

    // triggers menu/pause
    if (inputManager->isKeyPressed(GLFW_KEY_P)) {
        //gamePaused();
    }
}

// Input -> Movement
//==================================================================================================================//

void VehicleControlSystem::accelerate(float throttle)
{
    //logger::info("Accelerating...");
    // apply transformation here to move car forward

    auto& vehicle = gCoordinator.GetComponent<VehicleComponent>(playerVehicleEntity);
    vehicle.forwardGearDesired = true;

    if (vehicle.hasGearDesired()) {
        // currentThrottle set to 1.0 if flag isBoosting raised in this frame
        currentThrottle = vehicle.isBoosting ? 1.0f : throttle;
        currentBrake = 0.0f;
    } else {
        currentBrake = throttle;
        currentThrottle = 0.0f;
    }
}

void VehicleControlSystem::brake()
{
    //logger::info("Braking...");
    // apply transformation here to slow car down

    auto& vehicle = gCoordinator.GetComponent<VehicleComponent>(playerVehicleEntity);

    vehicle.forwardGearDesired = false;

    if (vehicle.hasGearDesired()) {
        currentThrottle = 1.0f;
    }
    else {
        currentBrake = 1.0f;
    }
}

void VehicleControlSystem::steerRight()
{
    //logger::info("Steer Right...");
    // apply transformation here to steer the car to the right
    currentSteer = -1.0f; // full right steer
}

void VehicleControlSystem::steerLeft()
{
    //logger::info("Steer Left...");
    // apply transformation here steer the car to the left
    currentSteer = 1.0f; // full left steer 
}

// Input -> Activate Skills
//==================================================================================================================//

void VehicleControlSystem::boost()
{
    //logger::info("Activate Boost...");
    // apply transformation here accelerate car even faster due to boost.
    // probably need a CD for this?
    //currentThrottle = 1.f; // full throttle
    //accelerate(1.0f);

    auto& vehicle = gCoordinator.GetComponent<VehicleComponent>(playerVehicleEntity);

    // Rising edge : instant boost cost 10% if not boosting before
    if (vehicle.timeSinceLastBoost > 0.1f) {
        vehicle.engineHeat = std::min(1.0f, vehicle.engineHeat + vehicle.boostHeatInstantCost);
        logger::info("BOOST START: Instant cost applied");
    }

    if (vehicle.engineHeat < 1.0f) {
        vehicle.isBoosting = true;
        accelerate(1.0f); // Force throttle max
    }
    else {
        accelerate();
    }
}

void VehicleControlSystem::throwSnowball()
{
    auto& vehicleComponent = gCoordinator.GetComponent<VehicleComponent>(playerVehicleEntity);
    if (vehicleComponent.snowBallCooldown > 0.f) return;

    auto& vehicleTransform = gCoordinator.GetComponent<PhysxTransform>(playerVehicleEntity);
    std::cout << "{" << vehicleTransform.pos.x << "f, "
        << vehicleTransform.pos.y << "f, "
        << vehicleTransform.pos.z << "f}" << std::endl;

    
    // 1. Safety Check: Ensure the player entity is valid
    if (!gCoordinator.HasComponent<VehicleComponent>(playerVehicleEntity)) return;

    //  ...and that the snow ball cool down is finished
    // auto& vehicleComponent = gCoordinator.GetComponent<VehicleComponent>(playerVehicleEntity);
    if (vehicleComponent.snowBallCooldown > 0.f) return;

    // play sound of throwing snowball
    audioManager->playSounds("assets/audio/snowball-hit-01.mp3", { 0,0,0 }, -1.0f);
    //logger::info("Throwing snowball...");

    // Calculate the Forward direction based on the vehicle's current rotation
    // In many coordinate systems, (0, 0, 1) is the local forward axis
    glm::vec3 forward = renderingSystem->getCameraForward();

    // Calculate Spawn Position: 
    // Offset the snowball so it doesn't spawn inside the car's collision box
    glm::vec3 spawnPos = vehicleTransform.pos + (forward * 3.0f) + glm::vec3(0, 2.0f, 0);

    // 3. Create Visual Entity
    Entity snowball = renderingSystem->createSphereEntity("assets/textures/snowball.png");

    auto& ballTrans = gCoordinator.GetComponent<PhysxTransform>(snowball);
    ballTrans.pos = spawnPos;
    ballTrans.rot = vehicleTransform.rot; // Align snowball orientation with the car

    // 4. Setup Physics
    float snowballRadius = 0.5f;
    ballTrans.scale = glm::vec3(snowballRadius);

    // Create the RigidBody and add it to the ECS
    gCoordinator.AddComponent(snowball, physicsSystem->createRigidBodyFromSphere(snowball, snowballRadius));

    // 5. Apply Initial Velocity
    physx::PxRigidDynamic* dynamicActor = gCoordinator.GetComponent<RigidBody>(snowball).actor->is<physx::PxRigidDynamic>();
    if (dynamicActor) {
        float launchSpeed = 90.f; // Meters per second
        glm::vec3 velocity = forward * launchSpeed;

        // Enable CCD (Continuous Collision Detection)
        dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);

        // Pass the velocity vector to PhysX
        dynamicActor->setLinearVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
        dynamicActor->setMass(dynamicActor->getMass() * 25);
    }
    vehicleComponent.snowBallCooldown = 3.0f;
}

// load basic vehicle sounds
void VehicleControlSystem::loadVehicleSounds()
{
    // for acceleration
    audioManager->loadSound("assets/audio/snowmobiles-4-trimmed.mp3", false, true, true);
    // for throwing snowball
    audioManager->loadSound("assets/audio/snowball-hit-01.mp3", false, false, false);
}
