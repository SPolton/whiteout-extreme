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
    void initGatesFromPoints();

    void restart();

    bool raceFinished = false;
    bool playerWinner = false;

    float totalRaceLength = 0.f;

    std::vector<Entity> leaderboard;
    
    const Gate* getGatePtr(size_t index) { return &gates.at(index); }

private:
    std::shared_ptr<RenderingSystem> renderingSystem;
    std::shared_ptr<PhysicsSystem> physicsSystem;

    float getDistanceToGateLine(const glm::vec3& racerPos, const Gate& gate);

    // List of gates hardcoded
    std::vector<Gate> gatesOld = {
        // Starting Block (Straight Section)
        {0, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 12.0f, glm::vec3{0.0f, 0.0f, 0.0f}},
        {1, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 12.0f, glm::vec3{0.0f, 0.0f, 20.0f}},
        {2, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 15.0f, glm::vec3{0.0f, 0.0f, 45.0f}},

        // Entry into High-Speed Curve (Slightly widening)
        {3, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 18.0f, glm::vec3{5.0f, 0.0f, 75.0f}},
        {4, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 20.0f, glm::vec3{15.0f, 0.0f, 100.0f}},

        // The Long Sweeping Turn (Apex)
        {5, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 25.0f, glm::vec3{35.0f, 0.0f, 125.0f}},
        {6, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 30.0f, glm::vec3{65.0f, 0.0f, 140.0f}},
        {7, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 30.0f, glm::vec3{100.0f, 0.0f, 145.0f}},

        // Exit toward Finish Line
        {8, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 25.0f, glm::vec3{130.0f, 0.0f, 135.0f}},
        {9, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 20.0f, glm::vec3{155.0f, 0.0f, 115.0f}},
        {10, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 15.0f, glm::vec3{170.0f, 0.0f, 90.0f}}
    };

    std::vector<Gate> gates = {
        // --- Start line (width 12.0) ---
        {0, {-6.0f, 0.0f, 0.0f},   {6.0f, 0.0f, 0.0f}},
        {1, {-6.0f, 0.0f, 20.0f},  {6.0f, 0.0f, 20.0f}},

        // --- Narrow turns (width 8.0) ---
        {2, {-15.0f, 0.0f, 40.0f}, {-7.0f, 0.0f, 38.0f}},
        {3, {-25.0f, 0.0f, 45.0f}, {-20.0f, 0.0f, 38.0f}},
        {4, {-35.0f, 0.0f, 55.0f}, {-30.0f, 0.0f, 48.0f}},

        // --- Transition ---
        {5, {-30.0f, 0.0f, 80.0f}, {-15.0f, 0.0f, 85.0f}},

        // --- Wide turn (width 25.0) ---
        {6, {0.0f, 0.0f, 100.0f},  {20.0f, 0.0f, 110.0f}},
        {7, {40.0f, 0.0f, 120.0f}, {65.0f, 0.0f, 130.0f}},
        {8, {80.0f, 0.0f, 110.0f}, {100.0f, 0.0f, 125.0f}},

        // --- Finish Line (width 15.0) ---
        {9, {110.0f, 0.0f, 80.0f}, {125.0f, 0.0f, 85.0f}},
        {10,{120.0f, 0.0f, 50.0f}, {135.0f, 0.0f, 55.0f}}
    };
};
