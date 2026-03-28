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

    //resetInputs();
    //processInputs();

    for (auto const& entity : mEntities) {
        auto& vehicle = gCoordinator.GetComponent<VehicleComponent>(entity);
        if (!vehicle.instance) {
            continue;
        }
        auto& engineParams = vehicle.instance->getVehicleData().mEngineDriveParams.engineParams;

        // Call inputs
        if (entity == playerVehicleEntity) {
            vehicle.isBoosting = false;
            resetInputs();
            processInputs();
        }

        if (vehicle.isOverheated) {
            vehicle.isBoosting = false;
        }

        if (vehicle.boostMaster) {
            vehicle.timeSinceBoostMaster += deltaTime;
            if (vehicle.timeSinceBoostMaster > 2.0f) {
                vehicle.boostMaster = false;
                vehicle.boostMasterBonus = 0.0f;
                vehicle.timeSinceBoostMaster = 0.0f;
            }
        }

        // on first time playing boost sound, start the audio file
        if (vehicle.isBoosting && firstTimePlaying) {
            boostChannelID = audioManager->playSounds("assets/audio/boost.wav", { 0,0,0 }, -7.f);
            firstTimePlaying = false;
        }
        // otherwise resume and pause boost channel based on vehicle boosting state
        else if (vehicle.isBoosting && !boostPlaying) {
            audioManager->resumeChannel(boostChannelID);
            boostPlaying = true;
        }
        else if (!vehicle.isBoosting) {
            audioManager->pauseChannel(boostChannelID);
            boostPlaying = false;
        }

        if (vehicle.isBoosting && !vehicle.isOverheated) {
            if (vehicle.boostMaster) vehicle.boostMaster = false;

            // --- LOGIC BOOST ---
            engineParams.maxOmega = 2100.0f;
            vehicle.engineHeat = std::min(1.0f, vehicle.engineHeat + vehicle.boostHeatPerSecond * deltaTime);
            vehicle.timeSinceLastBoost = 0.0f;

            // --- OVERHEAT ---
            if (vehicle.engineHeat >= 1.0f) {
                // pause boost sound when engine overheated
                audioManager->pauseChannel(boostChannelID);
                boostPlaying = false;

                if (entity == playerVehicleEntity) audioManager->playSounds("assets/audio/overheat.mp3", { 0,0,0 }, -6.0f);
                vehicle.engineHeat = 1.0f;
                vehicle.isOverheated = true;
                vehicle.isBoosting = false;
                vehicle.boostMaster = false;

                // Max Speed reduced punishment
                engineParams.maxOmega = 600.0f;
            
                // Slight spin and front impulse punishments (PHYSX)
                if (gCoordinator.HasComponent<RigidBody>(entity)) {
                    auto& rb = gCoordinator.GetComponent<RigidBody>(entity);
                    auto* dynamicActor = rb.actor->is<physx::PxRigidDynamic>();

                    if (dynamicActor) {
                        float spinImpulse = 6500.0f;
                        float sideSelect = (rand() % 2 == 0) ? 1.0f : -1.0f;
                        dynamicActor->addTorque(physx::PxVec3(0, spinImpulse * sideSelect, 0), physx::PxForceMode::eIMPULSE);
                        
                        physx::PxTransform pose = dynamicActor->getGlobalPose();
                        physx::PxVec3 backwardDir = -pose.q.rotate(physx::PxVec3(0, 0, 1));

                        float hitForce = 9000.0f;
                        physx::PxVec3 frontHitImpulse = (backwardDir * hitForce) + physx::PxVec3(0, 1000.0f, 0);

                        dynamicActor->addForce(frontHitImpulse, physx::PxForceMode::eIMPULSE);

                    }
                }
                logger::error("ENGINE OVERHEATED!");
            }
        }
        else {
            // --- LOGIC COOLING ---
            engineParams.maxOmega = vehicle.isOverheated? 700.f: 1300.0f;
            vehicle.timeSinceLastBoost += deltaTime;

            if (vehicle.isOverheated && vehicle.engineHeat <= 0.2f) {
                vehicle.isOverheated = false;
            }

            // - APEX-VENT -
            if (vehicle.engineHeat > 0.90f && !vehicle.isOverheated) {
                if (entity == playerVehicleEntity) audioManager->playSounds("assets/audio/apex-vent.mp3", { 0,0,0 }, -13.0f);
                vehicle.boostMaster = true;
                vehicle.timeSinceBoostMaster = 0.0f;

                float x = (vehicle.engineHeat - 0.90f) / (1.0f - 0.90f);
                vehicle.boostMasterAccuracy = x;

                vehicle.boostMasterBonus = 0.1f + (std::pow(x, 3.0f) * 0.4f);

                vehicle.engineHeat -= vehicle.boostMasterBonus;
                vehicle.timeSinceLastBoost = 0.75f;
            }

            if (vehicle.timeSinceLastBoost > 1.0f && vehicle.engineHeat > 0.0f) {
                float t = vehicle.timeSinceLastBoost - 1.2f;

                float decayRate = (0.03f * (t * t)) + 0.05f;

                if (vehicle.engineFreezing) {
                    decayRate *= 3.0f;
                    if (vehicle.isOverheated) {
                        decayRate *= 0.3f;
                    }
                }
                else if (vehicle.isOverheated) {
                    decayRate *= 0.5f;
                }

                vehicle.engineHeat -= decayRate * deltaTime;
                if (vehicle.engineHeat < 0.0f) vehicle.engineHeat = 0.0f;
            }
        }

        // LOGIC FREEZING
        if (vehicle.engineFreezing) {
            vehicle.engineHeat = std::clamp(vehicle.engineHeat - deltaTime * 0.08f, 0.0f, 1.0f);
            //if (entity == playerVehicleEntity) {
                //logger::warn("Avalanche is cooling down the engine");
            //}
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

    // get current state of boost button
    bool boostIsPressed = inputManager->isControllerButtonPressed(GLFW_GAMEPAD_BUTTON_Y);

    // if the button is currently pressed down and previously wasn't, play the nitro starting sound
    if (boostIsPressed && !boostWasPressedController) {
        audioManager->playSounds("assets/audio/nitro-start.wav", { 0,0,0 }, -8.0f);
    }
    // if the button is currently NOT pressed down and previously WAS, play the nitro ending sound
    else if (!boostIsPressed && boostWasPressedController) {
        // when ending boost, play fade out boost
        audioManager->playSounds("assets/audio/boost-end.wav", { 0,0,0 }, -7.f);
    }

    // if top button pressed, activate boost
    // if left button pressed, throw projectile
    if (boostIsPressed) {
        boost();
    }
    else if (inputManager->isControllerButtonPressed(GLFW_GAMEPAD_BUTTON_X)) {
        throwSnowball();
    }

    // update state to track for next frame
    boostWasPressedController = boostIsPressed;

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

    // get current state of boost button
    bool boostIsPressed = inputManager->isKeyPressed(GLFW_KEY_LEFT_SHIFT);

    // if the key is currently pressed down and previously wasn't, play the nitro starting sound
    if (boostIsPressed && !boostWasPressedKeybaord) {
        audioManager->playSounds("assets/audio/nitro-start.wav", { 0,0,0 }, -8.0f);
    }
    // if the key is currently NOT pressed down and previously WAS, play the nitro ending sound
    else if (!boostIsPressed && boostWasPressedKeybaord) {
        // when ending boost, play fade out boost
        audioManager->playSounds("assets/audio/boost-end.wav", { 0,0,0 }, -7.f);
    }

    // let us just assume we use one skill at a time
    if (boostIsPressed) {
        boost();
    }
    else if (inputManager->isKeyPressed(GLFW_KEY_SPACE) || inputManager->isKeyPressed(GLFW_KEY_E)) {
        throwSnowball();
    }

    // update state to track for next frame
    boostWasPressedKeybaord = boostIsPressed;

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
        currentThrottle = throttle;
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

    if (vehicle.isOverheated) {
        vehicle.isBoosting = false;
        return;
    }

    if (vehicle.engineHeat < 1.0f && !vehicle.isOverheated) {
        vehicle.isBoosting = true;
        accelerate(1.0f);
    }
    else {
        vehicle.isBoosting = false;
        accelerate();
    }

    if (vehicle.isBoosting && vehicle.timeSinceLastBoost > 0.1f) {
        vehicle.engineHeat = std::min(1.0f, vehicle.engineHeat + vehicle.boostHeatInstantCost());
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
    audioManager->playSounds("assets/audio/snowball-hit-01.mp3", { 0,0,0 }, 2.0f);
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
    audioManager->loadSound("assets/audio/snowmobile-player.wav", false, true, true);
    // for throwing snowball
    audioManager->loadSound("assets/audio/snowball-hit-01.mp3", false, false, false);
    // for boost
    audioManager->loadSound("assets/audio/boost.wav", false, true, true);
    audioManager->loadSound("assets/audio/nitro-start.wav", false, false, false);
    audioManager->loadSound("assets/audio/boost-end.wav", false, false, false);
}

// called from RacingGame to pause boost audio
void VehicleControlSystem::pauseBoostAudio()
{
    if (!audioManager) return;
    audioManager->pauseChannel(boostChannelID);
    boostPlaying = false;
}
