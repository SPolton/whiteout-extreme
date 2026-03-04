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
        glm::vec3 direction = glm::vec3(0.f, 0.f, 1.f);
        float initialSpeed;
        float baseSpeed = 0.5f;
        float maxSpeed = 1.f;
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

    // Set a new direction for the avalanche (will update physics rotation)
    void setDirection(const glm::vec3& newDirection);

    // Get list of engulfed players (as indices into the playerPositions array)
    const std::vector<size_t>& getEngulfedPlayerIndices() const { return mEngulfedPlayerIndices; }
    bool isPlayerEngulfed(size_t playerIndex) const;
    bool areAllPlayersEngulfed(size_t totalPlayers) const;

    // State
    glm::vec3 mPosition;
    glm::vec3 mSize;
    glm::vec3 mDirection;
    float mSpeed = 1.f;
    float mBaseSpeed;
    float mMaxSpeed;
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

    // Constants
    static constexpr float engulfThreshold = 3.0f;  // Distance to engulf player
    static constexpr float autoEngulfTime = 5.0f;   // Seconds before auto-engulf
    static constexpr float catchUpSpeedMultiplier = 1.5f;  // Speed increase when catching
};
