#pragma once

#include "components/VehicleComponent.h"
#include "input/glfw/InputManager.hpp"
#include <memory>
#include "ecs/Coordinator.hpp"
#include "core/RenderingSystem.hpp"
#include "physics/PhysicsSystem.hpp"
#include "audio/AudioEngine.h"

class VehicleControlSystem : public System {
public:
    VehicleControlSystem(std::shared_ptr<InputManager> inputManager,
        std::shared_ptr<AudioEngine> audioManager,
        std::shared_ptr<RenderingSystem> renderingSystem,
        std::shared_ptr<PhysicsSystem> physicsSystem);

    void update(float deltaTime);

    // Sound
    void loadVehicleSounds();
    int channelID;

private:
    // Input management
    std::shared_ptr<InputManager> inputManager;
    std::shared_ptr<AudioEngine> audioManager;
    std::shared_ptr<RenderingSystem> renderingSystem;
    std::shared_ptr<PhysicsSystem> physicsSystem;

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
    void accelerate(float throttle = 0.7f);
    void brake();
    void steerRight();
    void steerLeft();

    // Skills
    void boost();
    void throwSnowball();

    // keep track of boost sound playing state
    int boostChannelID = -1;
    bool boostPlaying = false;
    // one for each input type to prevent overlapping audio
    bool boostWasPressedController = false;
    bool boostWasPressedKeybaord = false;
};
