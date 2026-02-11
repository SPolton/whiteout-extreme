#pragma once

#include "components/VehicleComponent.h"
#include "ecs/System.hpp"
#include "input/InputManager.hpp"
#include <memory>

class VehicleControlSystem : public System {
public:
    void SetInputManager(std::shared_ptr<InputManager> inputManager);

    void update(float deltaTime);

private:
    // Input management
    std::shared_ptr<InputManager> inputManager;

    void processInputs();
    void processKeyboardInput();
    void processControllerInput();

    float currentThrottle = 0.0f;
    float currentBrake = 0.0f;
    float currentSteer = 0.0f;

    void resetInputs() {
        currentThrottle = 0.0f;
        currentBrake = 0.0f;
        currentSteer = 0.0f;
    }

    // Basic Movement
    void accelerate();
    void brake();
    void steerRight();
    void steerLeft();

    // Skills
    void boost();
};
