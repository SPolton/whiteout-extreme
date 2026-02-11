#include "VehicleControlSystem.hpp"
#include "ecs/Coordinator.hpp"
#include <GLFW/glfw3.h>
#include "utils/logger.h"

extern Coordinator gCoordinator;

void VehicleControlSystem::SetInputManager(std::shared_ptr<InputManager> EinputManager) {
    inputManager = EinputManager;
}

void VehicleControlSystem::update(float deltaTime) {
    if (!inputManager) return;

    resetInputs();
    processInputs();

    for (auto const& entity : mEntities) {
        auto& vehicle = gCoordinator.GetComponent<VehicleComponent>(entity);

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

void VehicleControlSystem::processInputs() {
    // poll controller state first
    inputManager->pollControllerInputs();

    // based on whether controller is connected, choose the correct input system
    if (inputManager->IsControllerConnected()) {
        processControllerInput();
    }
    else {
        processKeyboardInput();
    }
}


void VehicleControlSystem::processControllerInput() {
    /*
    * Right Trigger = accelerate
    * Left Trigger = brake
    * Left Stick = steering
    * X (switch) = Y (Xbox) = Nitro
    * Y (switch) = X (Xbox) = Throw Snowball (assuming auto aim for now...otherwise right stick input needed?)
    */

    // check for throttle/braking
    // anything greater than 0 means it is pressed
    if (inputManager->GetControllerAxis(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.0f) {
        accelerate();
    }
    else if (inputManager->GetControllerAxis(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER) > 0.0f) {
        brake();
    }

    // add "deadzone" (since most controllers won't return back to 0.0 exactly)
    float deadZone = 0.2f;

    // check for steering on left joystick
    // if positive, it is steering right
    // if negative, it is steering left
    if (inputManager->GetControllerAxis(GLFW_GAMEPAD_AXIS_LEFT_X) > deadZone) {
        steerRight();
    }
    else if (inputManager->GetControllerAxis(GLFW_GAMEPAD_AXIS_LEFT_X) < -deadZone) {
        steerLeft();
    }

    // if top button pressed, activate boost
    // if left button pressed, throw projectile
    if (inputManager->IsControllerButtonDown(GLFW_GAMEPAD_BUTTON_Y)) {
        boost();
    }
    else if (inputManager->IsControllerButtonDown(GLFW_GAMEPAD_BUTTON_X)) {
        //throwSnowball();
    }

    // triggers menu/pause
    if (inputManager->IsControllerButtonDown(GLFW_GAMEPAD_BUTTON_START)) {
        //gamePaused();
    }
}

void VehicleControlSystem::processKeyboardInput() {
    /*
    * WASD (or arrow keys) to move/steer
    * W (up arrow) = accelerate
    * S (down arrow) = brake
    * A (left arrow) = steer left
    * D (right arrow) = steer right
    */

    if (inputManager->IsKeyboardButtonDown(GLFW_KEY_W) || inputManager->IsKeyboardButtonDown(GLFW_KEY_UP)) {
        accelerate();
    }
    else if (inputManager->IsKeyboardButtonDown(GLFW_KEY_S) || inputManager->IsKeyboardButtonDown(GLFW_KEY_DOWN)) {
        brake();
    }

    // Should be able to steer left or right while accelerating
    if (inputManager->IsKeyboardButtonDown(GLFW_KEY_D) || inputManager->IsKeyboardButtonDown(GLFW_KEY_RIGHT)) {
        steerRight();
    }
    else if (inputManager->IsKeyboardButtonDown(GLFW_KEY_A) || inputManager->IsKeyboardButtonDown(GLFW_KEY_LEFT)) {
        steerLeft();
    }

    /*
    * Key Mappings for additional game functions
    * Left Shift = Nitro
    * Space Bar = Throw Snowball (assuming auto aim for now...otherwise mouse input needed)
    */

    // let us just assume we use one skill at a time
    if (inputManager->IsKeyboardButtonDown(GLFW_KEY_LEFT_SHIFT)) {
        boost();
    }
    else if (inputManager->IsKeyboardButtonDown(GLFW_KEY_SPACE)) {
        //throwSnowball();
    }

    // triggers menu/pause
    if (inputManager->IsKeyboardButtonDown(GLFW_KEY_P)) {
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
