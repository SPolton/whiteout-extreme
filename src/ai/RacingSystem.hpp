#pragma once

#include "components/Racer.h"
#include <memory>
#include "ecs/Coordinator.hpp"
#include "core/RenderingSystem.hpp"
#include "physics/PhysicsSystem.hpp"

class RacingSystem : public System {
public:
    RacingSystem(std::shared_ptr<RenderingSystem> renderingSystem,
        std::shared_ptr<PhysicsSystem> physicsSystem);

    void update(float deltaTime);

    void initGates();

    void restart();

    bool raceFinished = false;
    bool playerWinner = false;

    float totalRaceLength = 0.f;
    
    const Gate* getGatePtr(size_t index) { return &gates.at(index); }

private:
    std::shared_ptr<RenderingSystem> renderingSystem;
    std::shared_ptr<PhysicsSystem> physicsSystem;

    float getDistanceToGateLine(const glm::vec3& racerPos, const Gate& gate);

    // List of gates hardcoded
    std::vector<Gate> gates = {
        // Starting Block (Straight Section)
        {0, 12.0f, glm::vec3{0.0f, 0.0f, 0.0f}},
        {1, 12.0f, glm::vec3{0.0f, 0.0f, 20.0f}},
        {2, 15.0f, glm::vec3{0.0f, 0.0f, 45.0f}},

        // Entry into High-Speed Curve (Slightly widening)
        {3, 18.0f, glm::vec3{5.0f, 0.0f, 75.0f}},
        {4, 20.0f, glm::vec3{15.0f, 0.0f, 100.0f}},

        // The Long Sweeping Turn (Apex)
        {5, 25.0f, glm::vec3{35.0f, 0.0f, 125.0f}},
        {6, 30.0f, glm::vec3{65.0f, 0.0f, 140.0f}},
        {7, 30.0f, glm::vec3{100.0f, 0.0f, 145.0f}},

        // Exit toward Finish Line
        {8, 25.0f, glm::vec3{130.0f, 0.0f, 135.0f}},
        {9, 20.0f, glm::vec3{155.0f, 0.0f, 115.0f}},
        {10, 15.0f, glm::vec3{170.0f, 0.0f, 90.0f}} // Finish Line
    };
};
