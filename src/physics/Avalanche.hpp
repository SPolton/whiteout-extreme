#pragma once

#include "components/Transform.h"
#include "ecs/Types.hpp"

#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <vector>
#include <limits>

class Avalanche {
public:
    struct ConstructData {
        const char* name;
        glm::vec3 startPosition;
        float initialSpeed;
        float baseSpeed;
        float maxSpeed;
        float width;
        float height;
        float depth;
        physx::PxPhysics* physics;
        physx::PxScene* scene;
        physx::PxMaterial* material;
    };

    Avalanche(const ConstructData& data);

    // Update avalanche position and state
    void update(float deltaTime, const std::vector<glm::vec3>& playerPositions);

    // Get list of engulfed players (as indices into the playerPositions array)
    const std::vector<size_t>& getEngulfedPlayerIndices() const { return mEngulfedPlayerIndices; }
    bool isPlayerEngulfed(size_t playerIndex) const;

    // State
    glm::vec3 mPosition;
    glm::vec3 mSize;
    float mSpeed = 1.f;
    float mBaseSpeed = 1.f;
    float mMaxSpeed = 1.f;
    bool mIsActive = true;

    // Physics
    physx::PxRigidDynamic* mPhysicsActor = nullptr;
    physx::PxShape* mPhysicsShape = nullptr;

    // Gameplay - store indices instead of Entity IDs
    std::vector<size_t> mEngulfedPlayerIndices;

private:
    void initPhysicsActor(const ConstructData& data);
    void updatePosition(float deltaTime);
    void checkPlayerCollisions(const std::vector<glm::vec3>& playerPositions);
    void updateRubberbanding(const std::vector<glm::vec3>& playerPositions);

    // Time tracking for rubber-banding
    float mTimeLastPlayerInBack = 0.0f;
    size_t mLastPlayerIndex = 0;

    // Constants
    static constexpr float engulfThreshold = 3.0f;  // Distance to engulf player
    static constexpr float autoEngulfTime = 5.0f;   // Seconds before auto-engulf
    static constexpr float catchUpSpeedMultiplier = 1.5f;  // Speed increase when catching
};
