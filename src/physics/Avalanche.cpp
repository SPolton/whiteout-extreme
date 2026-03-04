#include "Avalanche.hpp"
#include "common/Flags.hpp"
#include "utils/logger.h"

Avalanche::Avalanche(const ConstructData& data)
    : mPosition(data.startPosition)
    , mSize(data.width, data.height, data.depth)
    , mSpeed(data.initialSpeed)
    , mBaseSpeed(data.baseSpeed)
    , mMaxSpeed(data.maxSpeed)
{
    initPhysicsActor(data);
    logger::info("Avalanche created at position ({}, {}, {})", data.startPosition.x, data.startPosition.y, data.startPosition.z);
}

void Avalanche::initPhysicsActor(const ConstructData& data)
{
    if (!data.physics || !data.scene || !data.material) {
        logger::error("Invalid ConstructData: null physics pointers");
        throw std::invalid_argument("ConstructData contains null physics pointers");
    }

    // Create a kinematic rigid body for the avalanche
    // Kinematic bodies don't respond to forces but can be moved by code
    physx::PxTransform pose(
        physx::PxVec3(mPosition.x, mPosition.y, mPosition.z),
        physx::PxQuat(physx::PxIdentity)
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
        // No players yet - move at base speed
        mSpeed = mBaseSpeed;
    }

    // Move avalanche forward
    updatePosition(deltaTime);
}

void Avalanche::updatePosition(float deltaTime)
{
    // Move forward in Z direction (match your coordinate system)
    mPosition.z += mSpeed * deltaTime;

    // Update physics actor position
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

    // Find the last player (furthest back in Z)
    size_t lastPlayerIndex = 0;
    float furthestZ = playerPositions[0].z;

    for (size_t i = 1; i < playerPositions.size(); ++i) {
        if (playerPositions[i].z < furthestZ) {
            furthestZ = playerPositions[i].z;
            lastPlayerIndex = i;
        }
    }

    // Calculate distance to last player
    float distanceToLastPlayer = playerPositions[lastPlayerIndex].z - mPosition.z;

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

        //todo
        //float distance = glm::distance(playerPositions[i], mPosition);
    }
}

bool Avalanche::isPlayerEngulfed(size_t playerIndex) const
{
    return std::find(mEngulfedPlayerIndices.begin(), mEngulfedPlayerIndices.end(), playerIndex) != mEngulfedPlayerIndices.end();
}
