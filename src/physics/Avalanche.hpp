#pragma once

#include "components/Racer.h"

#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

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
    //void update(float deltaTime, float distanceToLastPlayer, const std::vector<glm::vec3>& playerPositions);
    void stepPhysics(float deltaTime);
    //void updatePhysicsGeometry();
    // Set a new direction for the avalanche (will update physics rotation)
    void setDirection(const glm::vec3& newDirection);
    void setOrientation(const glm::vec3& lookAtTarget, float deltaTime);
    void adaptSpeed(float distanceToLastPlayer, float deltaTime, float firstRacerCompletion, float percentageToEngulfLastStandingRacer);

    Gate* gate; //next gate to be reached by the avalanche


    // Get list of engulfed players (as indices into the playerPositions array)
    //const std::vector<size_t>& getEngulfedPlayerIndices() const { return mEngulfedPlayerIndices; }
    //bool isPlayerEngulfed(size_t playerIndex) const;
    //bool areAllPlayersEngulfed(size_t totalPlayers) const;

    // State
    glm::vec3 mPosition;
    glm::vec3 mSize;
    glm::vec3 mDirection;
    glm::quat mRotation = glm::quat(1, 0, 0, 0);
    float mSpeed = 1.f;
    float mBaseSpeed;
    float mMaxSpeed;
    bool mIsActive = true;

    // Physics
    physx::PxRigidDynamic* mPhysicsActor = nullptr;
    physx::PxShape* mPhysicsShape = nullptr;

    // Gameplay - store indices instead of Entity IDs
    //std::vector<size_t> mEngulfedPlayerIndices;

    float mCloseProximityTimer = 0.0f;
    float raceCompletion = 0.0f;

private:
    void initPhysicsActor(const ConstructData& data);
    //void checkPlayerCollisions(const std::vector<glm::vec3>& playerPositions);
    //void updateRubberbanding(const std::vector<glm::vec3>& playerPositions);
    
    // Constants
    const float mDeathThresholdTime = 5.f;
    // const float mSafeDistance = 15.0f;
};
