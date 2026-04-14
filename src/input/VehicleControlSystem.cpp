#include "VehicleControlSystem.hpp"
#include "utils/logger.h"
//#include "app/RacingGame.cpp"

#include "gameplay/SnowBallisticSystem.hpp"

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
            boostChannelID = audioManager->jsonSound("vehicle.boost.loop");
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

                if (entity == playerVehicleEntity) {
                    overheatChannelID = audioManager->jsonSound("vehicle.overheat");
                }
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
                if (entity == playerVehicleEntity) {
                    apexVentChannelID = audioManager->jsonSound("vehicle.apex.vent");
                }
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
        if (vehicle.engineFreezing || vehicle.inSnowStream) {
            vehicle.engineHeat = std::clamp(vehicle.engineHeat - deltaTime * 0.08f, 0.0f, 1.0f);
            //if (entity == playerVehicleEntity) {
                //logger::warn("Avalanche is cooling down the engine");
            //}
        }

        // LOGIC IN SNOW STREAM
        if (vehicle.inSnowStream) {
            if (!vehicle.isBoosting) {
                vehicle.timeSinceLastBoost = glm::max(1.5f, vehicle.timeSinceLastBoost);
            }

            if (vehicle.timeInSnowStream == 0.0f) {
                vehicle.engineHeat -= 0.3f;
            }

            float burst = 0.27f; // (0.3 - 0.03)
            float decay = glm::exp(-2.0f * vehicle.timeInSnowStream); // fades in ~2sec
            float coolingPower = (burst * decay) + 0.03f;

            vehicle.engineHeat -= coolingPower * deltaTime; // always multiply by dt for frame-rate independence
            vehicle.timeInSnowStream += deltaTime;
        }
        else {
            vehicle.timeInSnowStream = 0.0f;
        }
        vehicle.inSnowStream = false;


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
            vehicle.instance->applyDriveCommand(
                currentThrottle,
                currentBrake,
                currentSteer,
                currentForwardGearDesired
            );
        }

        vehicle.visualSteer = currentSteer;

        // Update visual parts to follow vehicle
        if (vehicle.chassisVisual != 0) {
            auto& vehicleTransform = gCoordinator.GetComponent<PhysxTransform>(entity);
            float steerSpeed = 6.0f;
            vehicle.smoothedVisualSteer += (vehicle.visualSteer - vehicle.smoothedVisualSteer) * steerSpeed * deltaTime;
            vehicle.smoothedVisualSteer = glm::clamp(vehicle.smoothedVisualSteer, -1.0f, 1.0f);

            float maxSteerAngle = glm::radians(25.0f);
            float steeringAngle = vehicle.smoothedVisualSteer * maxSteerAngle;
            float handleSteeringAngle = steeringAngle * 0.3f;

            // Chassis follows vehicle exactly
            auto& chassisTrans = gCoordinator.GetComponent<PhysxTransform>(vehicle.chassisVisual);
            chassisTrans.pos = vehicleTransform.pos;
            chassisTrans.rot = vehicleTransform.rot;
            chassisTrans.scale = vehicleTransform.scale;

            // Handle
            glm::vec3 handleOffset = glm::vec3(0.0f, 1.43751f, -0.23f);
            glm::vec3 handleWorldOffset = vehicleTransform.rot * handleOffset;
            auto& handleTrans = gCoordinator.GetComponent<PhysxTransform>(vehicle.handleVisual);
            handleTrans.pos = vehicleTransform.pos + handleWorldOffset;
            handleTrans.rot = vehicleTransform.rot * glm::angleAxis(handleSteeringAngle, glm::vec3(0, 1, 0));
            handleTrans.scale = vehicleTransform.scale;

            // Left runner
            glm::vec3 runnerLOffset = glm::vec3(0.57f, 0.0f, 1.0f);
            glm::vec3 runnerLWorldOffset = vehicleTransform.rot * runnerLOffset;
            auto& runnerLTrans = gCoordinator.GetComponent<PhysxTransform>(vehicle.runnerLeftVisual);
            runnerLTrans.pos = vehicleTransform.pos + runnerLWorldOffset;
            runnerLTrans.rot = vehicleTransform.rot * glm::angleAxis(steeringAngle, glm::vec3(0, 1, 0));
            runnerLTrans.scale = vehicleTransform.scale;

            // Right runner
            glm::vec3 runnerROffset = glm::vec3(-0.57f, 0.0f, 1.0f);
            glm::vec3 runnerRWorldOffset = vehicleTransform.rot * runnerROffset;
            auto& runnerRTrans = gCoordinator.GetComponent<PhysxTransform>(vehicle.runnerRightVisual);
            runnerRTrans.pos = vehicleTransform.pos + runnerRWorldOffset;
            runnerRTrans.rot = vehicleTransform.rot * glm::angleAxis(steeringAngle, glm::vec3(0, 1, 0));
            runnerRTrans.scale = vehicleTransform.scale;
        }

        // update boost state
        isBoosting = vehicle.isBoosting;
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
    * Y, A, left bumper (Xbox) = Nitro
    * X, B (Xbox) = Throw Snowball
    */

    // check for throttle/braking
    // anything greater than 0 means it is pressed
    float throttleIsPressed = inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER);

    // if the key is currently NOT pressed down and previously WAS, play the engine ending sound
    if (!(throttleIsPressed > 0.0f) && (throttleWasPressedController > 0.0f)) {
        // when ending acceleration, play fade out engine
        engineEndChannelID = audioManager->jsonSound("vehicle.engine.end");
        // warn to stop looping engine sound
        stopPlayerEngine = true;
    }

    if (throttleIsPressed > 0.0f) {
        accelerate();
    }
    else if (inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER) > 0.0f) {
        brake();
    }

    // update state to track for next frame
    throttleWasPressedController = throttleIsPressed;

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
    // boost can be triggered by Y (top), A (bottom), and left bumper
    bool yPressed = inputManager->isControllerButtonPressed(GLFW_GAMEPAD_BUTTON_Y);
    bool aPressed = inputManager->isControllerButtonPressed(GLFW_GAMEPAD_BUTTON_A);
    bool leftBumperPressed = inputManager->isControllerButtonPressed(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER);

    // single variable to test if any of the three buttons are pressed
    bool boostIsPressed = (yPressed || aPressed || leftBumperPressed);

    // if the button is currently pressed down and previously wasn't, play the nitro starting sound
    if (boostIsPressed && !boostWasPressedController) {
        boostStartChannelID = audioManager->jsonSound("vehicle.boost.start");
    }
    // if the button is currently NOT pressed down and previously WAS, play the nitro ending sound
    else if (!boostIsPressed && boostWasPressedController) {
        // when ending boost, play fade out boost
        boostEndChannelID = audioManager->jsonSound("vehicle.boost.end");
    }

    // if boost button(s) pressed, activate boost
    if (boostIsPressed) {
        boost();

        // if no throttle, don't play engine
        if (!(throttleIsPressed > 0.0f)) {
            stopPlayerEngine = true;
        }
    }

    // if X (left button) or B (right button) is pressed, throw projectile
    bool xPressed = inputManager->isControllerButtonPressed(GLFW_GAMEPAD_BUTTON_X);
    bool bPressed = inputManager->isControllerButtonPressed(GLFW_GAMEPAD_BUTTON_B);
    if (xPressed || bPressed) {
        gCoordinator.GetSystem<SnowBallisticSystem>()->throwSnowball(playerVehicleEntity);
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

    // get current state of throttle key
    bool throttleIsPressed = inputManager->isKeyPressed(GLFW_KEY_W) || inputManager->isKeyPressed(GLFW_KEY_UP);

    // if the key is currently NOT pressed down and previously WAS, play the engine ending sound
    if (!throttleIsPressed && throttleWasPressedKeybaord) {
        // when ending acceleration, play fade out engine
        engineEndChannelID = audioManager->jsonSound("vehicle.engine.end");
        // warn to stop looping engine sound
        stopPlayerEngine = true;
    }

    if (throttleIsPressed) {
        accelerate();
    }
    else if (inputManager->isKeyPressed(GLFW_KEY_S) || inputManager->isKeyPressed(GLFW_KEY_DOWN)) {
        brake();
    }

    // update state to track for next frame
    throttleWasPressedKeybaord = throttleIsPressed;

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
        boostStartChannelID = audioManager->jsonSound("vehicle.boost.start");
    }
    // if the key is currently NOT pressed down and previously WAS, play the nitro ending sound
    else if (!boostIsPressed && boostWasPressedKeybaord) {
        // when ending boost, play fade out boost
        boostEndChannelID = audioManager->jsonSound("vehicle.boost.end");
    }

    // can boost and throw at the same time
    if (boostIsPressed) {
        boost();

        // if no throttle, don't play engine
        if (!throttleIsPressed) {
            stopPlayerEngine = true;
        }
    }

    if (inputManager->isKeyPressed(GLFW_KEY_SPACE) || inputManager->isKeyPressed(GLFW_KEY_E)) {
        gCoordinator.GetSystem<SnowBallisticSystem>()->throwSnowball(playerVehicleEntity);
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

    currentForwardGearDesired = true;
    currentThrottle = throttle;
    currentBrake = 0.0f;

    // is playing when accelerating
    stopPlayerEngine = false;
}

void VehicleControlSystem::brake()
{
    //logger::info("Braking...");
    // apply transformation here to slow car down

    currentForwardGearDesired = false;
    currentThrottle = 1.0f;
    currentBrake = 0.0f;
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

// load basic vehicle sounds
void VehicleControlSystem::loadVehicleSounds()
{
    audioManager->loadSoundRegistry();
}

// called from RacingGame to pause boost and engine audio
void VehicleControlSystem::pauseBoostAndEngineAudio()
{
    if (!audioManager) return;
    audioManager->pauseChannel(boostChannelID);
    audioManager->pauseChannel(boostStartChannelID);
    audioManager->pauseChannel(boostEndChannelID);
    audioManager->pauseChannel(overheatChannelID);
    audioManager->pauseChannel(apexVentChannelID);
    audioManager->pauseChannel(engineEndChannelID); // pause engine fade out as well
    boostPlaying = false;
}

// called from RacingGame to resume boost and engine audio
void VehicleControlSystem::resumeBoostAndEngineAudio()
{
    if (!audioManager) return;
    audioManager->resumeChannel(boostStartChannelID);
    audioManager->resumeChannel(boostEndChannelID);
    audioManager->resumeChannel(overheatChannelID);
    audioManager->resumeChannel(apexVentChannelID);
    // no need to deal with main boost sound, it is handled by key press per frame
    audioManager->resumeChannel(engineEndChannelID); // resume engine fade out as well
}
