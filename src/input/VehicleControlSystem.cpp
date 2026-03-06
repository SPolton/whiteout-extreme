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

        if (vehicle.snowBallCooldown > 0.f) vehicle.snowBallCooldown -= deltaTime;

        if(vehicle.playerID == 0) {
            vehicle.throttle = currentThrottle;
            vehicle.brake = currentBrake;
            vehicle.steer = currentSteer;

            if (vehicle.instance) {
                vehicle.instance->setInputs(
                    vehicle.throttle,
                    vehicle.brake,
                    vehicle.steer
                );
            }
        }
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

void VehicleControlSystem::accelerate()
{
    logger::info("Accelerating...");
    // apply transformation here to move car forward
    currentThrottle = 0.7f; // full throttle
}

void VehicleControlSystem::brake()
{
    logger::info("Braking...");
    // apply transformation here to slow car down
    currentBrake = 1.0f; // full brake
}

void VehicleControlSystem::steerRight()
{
    logger::info("Steer Right...");
    // apply transformation here to steer the car to the right
    currentSteer = -1.0f; // full right steer
}

void VehicleControlSystem::steerLeft()
{
    logger::info("Steer Left...");
    // apply transformation here steer the car to the left
    currentSteer = 1.0f; // full left steer 
}

// Input -> Activate Skills
//==================================================================================================================//

void VehicleControlSystem::boost()
{
    logger::info("Activate Boost...");
    // apply transformation here accelerate car even faster due to boost.
    // probably need a CD for this?
    currentThrottle = 1.f; // full throttle
}

void VehicleControlSystem::throwSnowball()
{
    // 1. Safety Check: Ensure the player entity is valid
    if (!gCoordinator.HasComponent<VehicleComponent>(playerVehicleEntity)) return;

    //  ...and that the snow ball cool down is finished
    auto& vehicleComponent = gCoordinator.GetComponent<VehicleComponent>(playerVehicleEntity);
    if (vehicleComponent.snowBallCooldown > 0.f) return;

    // ...and that the current camera is the TurnTable one
    if (!renderingSystem->isTurnTableCamera()) return;

    // play sound of throwing snowball
    audioManager->playSounds("assets/audio/snowball-hit-01.mp3", { 0,0,0 }, -1.0f);
    logger::info("Throwing snowball...");

    // 2. Retrieve Player Transform
    auto& vehicleTransform = gCoordinator.GetComponent<PhysxTransform>(playerVehicleEntity);

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
    float snowballRadius = 0.2f;
    ballTrans.scale = glm::vec3(snowballRadius);

    // Create the RigidBody and add it to the ECS
    gCoordinator.AddComponent(snowball, physicsSystem->createRigidBodyFromSphere(snowball, snowballRadius));

    // 5. Apply Initial Velocity
    physx::PxRigidDynamic* dynamicActor = gCoordinator.GetComponent<RigidBody>(snowball).actor->is<physx::PxRigidDynamic>();
    if (dynamicActor) {
        float launchSpeed = 30.f; // Meters per second
        glm::vec3 velocity = forward * launchSpeed;

        // Enable CCD (Continuous Collision Detection)
        dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);

        // Pass the velocity vector to PhysX
        dynamicActor->setLinearVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
        dynamicActor->setMass(dynamicActor->getMass() * 15);
    }
    vehicleComponent.snowBallCooldown = 0.5f;
}

// load basic vehicle sounds
void VehicleControlSystem::loadVehicleSounds()
{
    // for acceleration
    audioManager->loadSound("assets/audio/snowmobiles-4-trimmed.mp3", false, true, true);
    // for throwing snowball
    audioManager->loadSound("assets/audio/snowball-hit-01.mp3", false, false, false);
}
