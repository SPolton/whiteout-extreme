#include "Avalanche.hpp"
#include "common/Flags.hpp"
#include "utils/logger.h"

#include <cmath>

Avalanche::Avalanche(const ConstructData& data)
    : mPosition(data.startPosition)
    , mSize(data.width, data.height, data.depth)
    , mDirection(glm::normalize(data.direction))
    , mSpeed(data.initialSpeed)
    , mBaseSpeed(data.baseSpeed)
    , mMaxSpeed(data.maxSpeed)
{
    initPhysicsActor(data);
    logger::info("Avalanche created at position ({}, {}, {}) with direction ({}, {}, {})", 
                 data.startPosition.x, data.startPosition.y, data.startPosition.z,
                 mDirection.x, mDirection.y, mDirection.z);
}

void Avalanche::initPhysicsActor(const ConstructData& data)
{
    if (!data.physics || !data.scene || !data.material) {
        logger::error("Invalid ConstructData: null physics pointers");
        throw std::invalid_argument("ConstructData contains null physics pointers");
    }

    // Calculate rotation to align box with direction vector
    // Default PhysX box faces +Z, we want it to face our direction
    glm::vec3 defaultForward(0.f, 0.f, 1.f);
    glm::quat rotation = glm::rotation(defaultForward, mDirection);

    // Create a kinematic rigid body for the avalanche
    // Kinematic bodies don't respond to forces but can be moved by code
    physx::PxTransform pose(
        physx::PxVec3(mPosition.x, mPosition.y, mPosition.z),
        physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)
    );

    mPhysicsActor = data.physics->createRigidDynamic(pose);
    if (!mPhysicsActor) {
        logger::error("Failed to create avalanche physics actor");
        throw std::runtime_error("Failed to create avalanche physics actor");
    }

    // Make it kinematic (not affected by forces)
    mPhysicsActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);

    // Create a box shape for the avalanche
    mPhysicsShape = data.physics->createShape(
        physx::PxBoxGeometry(mSize.x / 2.0f, mSize.y / 2.0f, mSize.z / 2.0f),
        *data.material
    );

    if (!mPhysicsShape) {
        logger::error("Failed to create avalanche physics shape");
        throw std::runtime_error("Failed to create avalanche physics shape");
    }

    // Set collision flags
    mPhysicsShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
    mPhysicsShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
    mPhysicsShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);

    // Set collision filtering
    physx::PxFilterData filterData;
    filterData.word0 = COLLISION_FLAG_AVALANCHE;
    filterData.word1 = COLLISION_FLAG_AVALANCHE_AGAINST;
    mPhysicsShape->setSimulationFilterData(filterData);
    mPhysicsShape->setQueryFilterData(filterData);

    // Attach shape to actor
    mPhysicsActor->attachShape(*mPhysicsShape);
    mPhysicsShape->release();

    // Add to scene
    data.scene->addActor(*mPhysicsActor);

    logger::info("Avalanche physics actor initialized: size=({}, {}, {})", mSize.x, mSize.y, mSize.z);
}

void Avalanche::update(float deltaTime, const std::vector<glm::vec3>& playerPositions)
{
    if (!mIsActive) {
        return;
    }

    // Update speed based on player positions (rubber-banding)
    // Do this even if playerPositions is empty (avalanche keeps moving at base speed)
    if (!playerPositions.empty()) {
        updateRubberbanding(playerPositions);
        checkPlayerCollisions(playerPositions);
    } else {
        // No players yet move at base speed
        mSpeed = mBaseSpeed;
    }

    // Move avalanche forward
    updatePosition(deltaTime);
}

void Avalanche::setDirection(const glm::vec3& newDirection)
{
    // Normalize and store the new direction
    mDirection = glm::normalize(newDirection);

    // Update the physics actor rotation to match the new direction
    if (mPhysicsActor) {
        glm::vec3 defaultForward(0.f, 0.f, 1.f);
        glm::quat rotation = glm::rotation(defaultForward, mDirection);

        // Get current position and update with new rotation
        physx::PxTransform currentPose = mPhysicsActor->getGlobalPose();
        physx::PxTransform newPose(
            currentPose.p,
            physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)
        );
        mPhysicsActor->setKinematicTarget(newPose);

        logger::info("Avalanche direction changed to ({}, {}, {})", mDirection.x, mDirection.y, mDirection.z);
    }
}

void Avalanche::updatePosition(float deltaTime)
{
    // Move in the direction vector
    mPosition += mDirection * mSpeed * deltaTime;

    // Update physics actor position (keep the rotation)
    if (mPhysicsActor) {
        physx::PxTransform newPose(
            physx::PxVec3(mPosition.x, mPosition.y, mPosition.z),
            mPhysicsActor->getGlobalPose().q
        );
        mPhysicsActor->setKinematicTarget(newPose);
    }
}

void Avalanche::updateRubberbanding(const std::vector<glm::vec3>& playerPositions)
{
    if (playerPositions.empty()) {
        return;
    }

    // Find the last player (furthest back along direction vector)
    size_t lastPlayerIndex = 0;
    float furthestDistance = glm::dot(playerPositions[0] - mPosition, mDirection);

    for (size_t i = 1; i < playerPositions.size(); ++i) {
        float distance = glm::dot(playerPositions[i] - mPosition, mDirection);
        if (distance < furthestDistance) {
            furthestDistance = distance;
            lastPlayerIndex = i;
        }
    }

    // Calculate distance to last player along the direction axis
    float distanceToLastPlayer = glm::dot(playerPositions[lastPlayerIndex] - mPosition, mDirection);

    // Increase speed to maintain pressure
    if (distanceToLastPlayer < 50.0f && distanceToLastPlayer > 0.0f) {
        // Catch-up mode
        float speedMultiplier = 1.0f + (1.0f - (distanceToLastPlayer / 50.0f)) * catchUpSpeedMultiplier;
        mSpeed = mBaseSpeed * speedMultiplier;
    } else {
        // Normal pursuit
        mSpeed = mBaseSpeed;
    }

    // Clamp speed
    mSpeed = glm::clamp(mSpeed, mBaseSpeed, mMaxSpeed);
}

void Avalanche::checkPlayerCollisions(const std::vector<glm::vec3>& playerPositions)
{
    for (size_t i = 0; i < playerPositions.size(); ++i) {
        if (isPlayerEngulfed(i)) {
            continue;  // Already engulfed
        }

        // Get the player position in world space
        glm::vec3 playerPos = playerPositions[i];
        
        // Calculate the rotation quaternion from direction
        glm::vec3 defaultForward(0.f, 0.f, 1.f);
        glm::quat avalancheRotation = glm::rotation(defaultForward, mDirection);
        
        // Transform player position into avalanche's local space
        // Translate to avalanche center
        glm::vec3 relativePos = playerPos - mPosition;
        
        // Rotate by inverse of avalanche rotation to get local coordinates
        glm::quat invRotation = glm::inverse(avalancheRotation);
        glm::vec3 localPos = invRotation * relativePos;
        
        // Check if player is inside the oriented bounding box (in local space)
        glm::vec3 halfSize = mSize / 2.0f;
        bool insideX = std::abs(localPos.x) < halfSize.x;
        bool insideY = std::abs(localPos.y) < halfSize.y;
        bool insideZ = std::abs(localPos.z) < halfSize.z;
        
        if (insideX && insideY && insideZ) {
            // Player is engulfed!
            mEngulfedPlayerIndices.push_back(i);
            logger::warn("Player {} engulfed by avalanche at position ({}, {}, {})", 
                        i, playerPos.x, playerPos.y, playerPos.z);
        }
    }
}

bool Avalanche::isPlayerEngulfed(size_t playerIndex) const
{
    return std::find(mEngulfedPlayerIndices.begin(), mEngulfedPlayerIndices.end(), playerIndex) != mEngulfedPlayerIndices.end();
}

bool Avalanche::areAllPlayersEngulfed(size_t totalPlayers) const {
    return totalPlayers > 0 && mEngulfedPlayerIndices.size() >= totalPlayers;
}
