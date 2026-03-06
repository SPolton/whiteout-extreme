#pragma once

#include "physics/Avalanche.hpp"
#include "components/Racer.h"
#include "components/Physics.hpp"
#include <memory>
#include "ecs/Coordinator.hpp"
#include "core/RenderingSystem.hpp"
#include "physics/PhysicsSystem.hpp"

class RacingSystem : public System {
public:
    RacingSystem(std::shared_ptr<RenderingSystem> renderingSystem,
        std::shared_ptr<PhysicsSystem> physicsSystem);

    void update(float deltaTime);

    void init(std::shared_ptr<Avalanche> avalanche);

    int numberOfRacers();
    int numberOfEngulfedRacers();
    Entity getFirstRacerEntity();
    Entity getLastNonEngulfedRacerEntity();
    void refreshLeaderboard();

    void restart();

    bool raceFinished = false;
    bool playerWinner = false;

    float totalRaceLength = 0.f;

    std::vector<Entity> leaderboard;

    const Gate* getGatePtr(size_t index) { return &gates.at(index); }

private:
    std::shared_ptr<RenderingSystem> renderingSystem;
    std::shared_ptr<PhysicsSystem> physicsSystem;
    std::shared_ptr<Avalanche> avalanche;

    float getDistanceToGateLine(const glm::vec3& racerPos, const Gate& gate);

    void checkRacerEngulfment(Racer& racer, PhysxTransform& racerTransf);

    void initGatesFromPoints();

    //-730.0f, 670.4f, -400.0f;

    std::vector<Gate> gates = {
        // Gate 0 (G0, D0)
        {0, {-639.616f, 643.366f, -409.404f}, {-679.635f, 643.556f, -326.821f}},
        {1, {-629.616f, 643.366f, -399.404f}, {-669.635f, 643.556f, -316.821f}},
        // Gate 1 (G1, D1) 
        {2, {-533.285f, 633.838f, -329.626f}, {-567.817f, 634.495f, -248.743f}},
        // Gate 2 (G2, D2)
        {3, {-467.116f, 628.703f, -301.075f}, {-502.719f, 629.241f, -212.333f}},
     
        // Gate 3 (G3, D3) 
        {3, {-415.266f, 622.665f, -292.842f}, {-411.525f, 622.806f, -208.883f}},
        // Gate 4 (G4, D4)
        {4, {-383.695f, 618.184f, -300.967f}, {-329.536f, 615.337f, -250.635f}},
        // Gate 5 (G5, D5) 
        {5, {-331.843f, 610.418f, -322.291f}, {-272.391f, 609.843f, -285.001f}},
        // Gate 6 (G6, D6)
        {6, {-331.342f, 604.286f, -351.591f}, {-263.367f, 593.323f, -414.284f}},
        // Gate 7 (G7, D7) 
        {7, {-358.449f, 591.647f, -401.508f}, {-326.783f, 585.488f, -457.507f}},
        // Gate 8 (G8, D8)
        {8, {-448.590f, 582.782f, -392.834f}, {-455.013f, 577.641f, -461.336f}},
        // Gate 9 (G9, D9) 
        {9, {-533.581f, 572.891f, -408.376f}, {-492.569f, 574.252f, -469.492f}},
        // Gate 10 (G10, D10)
        {10, {-584.388f, 566.331f, -452.108f}, {-516.999f, 568.006f, -491.846f}},
        // Gate 11 (G11, D11) 
        {11, {-593.468f, 559.705f, -542.141f}, {-524.884f, 560.099f, -520.108f}},
        // Gate 12 (G12, D12)
        {12, {-574.433f, 555.347f, -586.799f}, {-519.216f, 555.705f, -543.662f}},
        // Gate 13 (G13, D13) 
        {13, {-545.868f, 550.625f, -611.163f}, {-507.278f, 551.109f, -557.854f}},
        // Gate 14 (G14, D14)
        {14, {-494.684f, 546.169f, -635.499f}, {-464.182f, 541.634f, -575.540f}},
        // Gate 15 (G15, D15) 
        /*
        {15, {-385.398f, 532.352f, -562.711f}, {-414.223f, 535.669f, -573.397f}},
        // Gate 16 (G16, D16)
        {16, {-362.272f, 531.443f, -624.854f}, {-398.287f, 535.337f, -638.127f}},
        // Gate 17 (G17, D17) 
        {17, {-246.839f, 519.193f, -460.053f}, {-274.851f, 522.185f, -485.422f}},
        // Gate 18 (G18, D18)
        {18, {-235.497f, 515.325f, -526.631f}, {-201.889f, 511.451f, -498.913f}},
        // Gate 19 (G19, D19) 
        {19, {-204.299f, 508.736f, -359.224f}, {-218.975f, 515.768f, -409.313f}},
        // Gate 20 (G20, D20)
        {20, {-155.525f, 506.474f, -415.374f}, {-145.740f, 501.928f, -367.035f}},
        // Gate 21 (G21, D21) 
        //{21, {-210.440f, 484.018f, -183.736f}, {-215.628f, 494.332f, -251.780f}},
        // Gate 22 (G22, D22)
        //{22, {-146.384f, 487.370f, -255.556f}, {-149.338f, 480.857f, -213.564f}},
        // Gate 23 (G23, D23)
        //{23, {-144.125f, 475.135f, -179.939f}, {-199.113f, 476.513f, -124.177f}},
        // Gate 24 (G24, D24)
        //{24, {-113.976f, 468.574f, -176.584f}, {-92.021f, 455.907f, -100.558f}}
        */
    };

/*
    // List of gates hardcoded
    std::vector<Gate> gates = {
        // --- Start line (width 12.0) ---
        {0, {-730.0f, 652.f, -400.0f},   {-730.0f, 652.f, -420.0f}},
        {1, {-680.0f, 652.f, -400.0f},   {-680.0f, 652.f, -420.0f}},
        {2, {-630.0f, 652.f, -400.0f},   {-630.0f, 652.f, -420.0f}},
        {3, {-580.0f, 652.f, -400.0f},   {-580.0f, 652.f, -420.0f}}

        
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
 */
    /*
    // List of gates hardcoded
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
    */

    /*
    void initGates();
 
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
    */
};
