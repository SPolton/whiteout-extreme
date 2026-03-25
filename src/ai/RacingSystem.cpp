#include "RacingSystem.hpp"
#include "utils/logger.h"
#include "utils/math.hpp"
#include <glm/gtx/projection.hpp>

#include <GLFW/glfw3.h>

extern Coordinator gCoordinator;
extern Entity playerVehicleEntity;

RacingSystem::RacingSystem(
    std::shared_ptr<RenderingSystem> renderingSystem,
    std::shared_ptr<PhysicsSystem> physicsSystem)
    : renderingSystem(renderingSystem),
    physicsSystem(physicsSystem)
{
}

void RacingSystem::update(float deltaTime)
{
    static float logTimer = 0.0f;
    logTimer += deltaTime;
    bool shouldLog = false;

    if (logTimer >= 1.f) {
        shouldLog = true;
        logTimer = 0.0f;
    }

    ///===== RACER LOGIC ====================
    for (auto const& entity : mEntities) {
        auto& racer = gCoordinator.GetComponent<Racer>(entity);
        auto& racerTransf = gCoordinator.GetComponent<PhysxTransform>(entity);
        auto& racerVehicle = gCoordinator.GetComponent<VehicleComponent>(entity);

        if (!racer.targetGate) {
            racer.raceCompletion = 1.0f;
            if (numberOfRacers() - numberOfEngulfedRacers() == 1) {
                raceFinished = true;
            }
            continue;
        }

        if (!racer.lastGate) {
            continue;
        }

        if(!racer.engulfed) checkRacerEngulfment(racer, racerTransf);
        if (racer.engulfed) {
            continue;
        }

        if (!racerVehicle.instance) {
            continue;
        }

        auto verticalVelocity = racerVehicle.instance->getRigidActor()->is<physx::PxRigidBody>()->getLinearVelocity().y;
        if (verticalVelocity < -25.0f) {
            racer.engulfed = true;
            logger::warn("Racer #{} fell off the cliff!", racer.currentRank);
            continue;
        }
        
        float distTarget = getDistanceToGateLine(racerTransf.pos, *racer.targetGate);
        float distLast = getDistanceToGateLine(racerTransf.pos, *racer.lastGate);

        float lengthOnLane = distLast / (distLast + distTarget) * racer.lastGate->laneLength;
        racer.lengthOnLane = lengthOnLane;
        racer.raceCompletion = (racer.lastGate->raceLength + lengthOnLane) / totalRaceLength;

        if (shouldLog && false) {
            // LOGGER INFO BLOCK
            logger::info("--- Racer Debug ---");
            logger::info("Racer Pos: ({:.2f}, {:.2f}, {:.2f})", racerTransf.pos.x, racerTransf.pos.y, racerTransf.pos.z);
            logger::info("Target Pos: ({:.2f}, {:.2f}, {:.2f})", racer.targetGate->position.x, racer.targetGate->position.y, racer.targetGate->position.z);
            logger::info("Dist to Gate Ligne (PS Length): {:.2f}", distTarget);
            logger::info("Length on current Lane: {:.2f} / {:.2f}", lengthOnLane, racer.lastGate->laneLength);
            logger::info("Race Completion: {:.2f}%", racer.raceCompletion * 100.0f);
            logger::info("-------------------");
        }

        // --- Gate logic ---
        glm::vec3 racerPosFlat{ racerTransf.pos.x, 0.0f, racerTransf.pos.z };
        glm::vec3 targetPosFlat{ racer.targetGate->position.x, 0.0f, racer.targetGate->position.z };
        glm::vec3 lastPosFlat{ racer.lastGate->position.x, 0.0f, racer.lastGate->position.z };

        float dotTarget = glm::dot(targetPosFlat - racerPosFlat, racer.targetGate->direction);
        float dotLast = glm::dot(lastPosFlat - racerPosFlat, racer.lastGate->direction);

        // A. FORWARD : checks if racer reached next gate
        if (dotTarget < 0.0f) {
                racer.lastGate = racer.targetGate;
                racer.targetGate = racer.targetGate->nextGate;
        }
        // B. BACKWARD : checks if racer got back behind last gate
        else if (dotLast > 0.0f) {
            if (racer.lastGate->prevGate) {
                racer.targetGate = racer.lastGate;
                racer.lastGate = racer.lastGate->prevGate;
            }
        }
    }

    this->refreshLeaderboard();

    ///===== AVALANCHE LOGIC ====================
    if(avalanche->mIsActive){
        glm::vec3 avalanchePos = avalanche->mPosition;

        Entity firstRacerEntity = getFirstRacerEntity();
        Entity lastRacerEntity = getLastNonEngulfedRacerEntity();

        glm::vec3 directionToNextGate;

        // --- Gate logic ---
    
        glm::vec3 avPosFlat{ avalanchePos.x, 0.0f, avalanchePos.z };
        glm::vec3 targetPosFlat{ avalanche->gate->position.x, 0.0f, avalanche->gate->position.z };
        glm::vec3 lastPosFlat{ avalanche->gate->prevGate->position.x, 0.0f, avalanche->gate->prevGate->position.z };

        float dotTarget = glm::dot(targetPosFlat - avPosFlat, avalanche->gate->direction);
        //float dotLast = glm::dot(lastPosFlat - avPosFlat, avalanche->gate->prevGate->direction);

        // A. FORWARD : checks if avalanche reached next gate
        if (dotTarget < 0.0f) {
            if (avalanche->gate->nextGate) {
                avalanche->gate = avalanche->gate->nextGate;
                logger::info("Next Gate for avalanche!");
            }
            else {
                avalanche->mIsActive = false;
            }
        }

        // DIRECTION
        directionToNextGate = avalanche->gate->position - avalanchePos;
        avalanche->setDirection(directionToNextGate);

        // ORIENTATION
        if (avalanche->gate->prevGate == &gates.at(0)) {
            glm::vec3 lookDirCircuit = avalanche->gate->prevGate->forward;
            avalanche->setOrientation(lookDirCircuit, deltaTime);
            //logger::error("avalanche race completion at {}", avalanche->raceCompletion);
            avalanche->raceCompletion = 0.0f;
        }
        else {
            // 1. Compute progression in current lane
            float segmentLen = avalanche->gate->prevGate->laneLength;
            float lengthOnSegment = 10.0f + glm::length(avalanchePos - avalanche->gate->prevGate->position);
            avalanche->raceCompletion = (avalanche->gate->prevGate->raceLength + lengthOnSegment) / totalRaceLength;
            //logger::error("avalanche race completion at {}", avalanche->raceCompletion);

            float progress = (lengthOnSegment-10.0f) / segmentLen;
            progress = glm::clamp(progress, 0.0f, 1.0f);

            // 2. Orientation via LERP between direction of last and next gate
            glm::vec3 lookDirCircuit = glm::normalize(glm::mix(
                avalanche->gate->prevGate->forward,
                avalanche->gate->forward,
                progress
            ));
            avalanche->setOrientation(lookDirCircuit, deltaTime);
        }

        // SPEED
        int NRacers = numberOfRacers();
        int NEngulfedRacers = numberOfEngulfedRacers();
        int NStandingRacers = NRacers - NEngulfedRacers;
        if (NRacers == NEngulfedRacers) {
            raceFinished = true;
        }

        float percentageToEngulfLastStandingRacer = NStandingRacers == 1? 1.0f:
            1.0f - 1.0f / ((NEngulfedRacers+1) * 2);

        float firstRacerCompletion = gCoordinator.GetComponent<Racer>(firstRacerEntity).raceCompletion;

        auto& lastRacer = gCoordinator.GetComponent<Racer>(lastRacerEntity);

        float distanceToLastRacer = (lastRacer.raceCompletion - avalanche->raceCompletion) * this->totalRaceLength; //* 100.f * percentagePerUnit;
        //logger::info("distanceToLastRacer: {}", distanceToLastRacer);

        avalanche->adaptSpeed(distanceToLastRacer, deltaTime, firstRacerCompletion, percentageToEngulfLastStandingRacer);
    }
}


int RacingSystem::numberOfRacers() {
    return static_cast<int>(leaderboard.size());
}


int RacingSystem::numberOfEngulfedRacers() {
    int result = 0;
    for (int i = static_cast<int>(leaderboard.size()) - 1; i >= 0; i--)
    {
        Entity currentEntity = leaderboard.at(i);
        auto& currentRacer = gCoordinator.GetComponent<Racer>(currentEntity);

        if (currentRacer.engulfed) {
            result++;
        }
    }
    return result;
}

Entity RacingSystem::getFirstRacerEntity(){
    return leaderboard.at(0);
}

Entity RacingSystem::getLastNonEngulfedRacerEntity() {

    Entity lastRacerEntity = this->leaderboard.at(leaderboard.size() - 1);
    for (int i = static_cast<int>(leaderboard.size()) - 1; i >= 0; i--)
    {
        Entity currentEntity = leaderboard.at(i);
        auto& currentRacer = gCoordinator.GetComponent<Racer>(currentEntity);

        if (!currentRacer.engulfed) {
            lastRacerEntity = currentEntity;
            break;
        }
    }
    return lastRacerEntity;
}

void RacingSystem::refreshLeaderboard() {
    leaderboard.assign(mEntities.begin(), mEntities.end());

    std::sort(leaderboard.begin(), leaderboard.end(), [](Entity a, Entity b) {
        auto& racerA = gCoordinator.GetComponent<Racer>(a);
        auto& racerB = gCoordinator.GetComponent<Racer>(b);

        return racerA.raceCompletion > racerB.raceCompletion;
        });

    for (size_t i = 0; i < leaderboard.size(); ++i) {
        gCoordinator.GetComponent<Racer>(leaderboard[i]).currentRank = static_cast<int>(i + 1);
    }
}

void RacingSystem::restart() {
    if (gates.empty()) return;

    // Reset booleans 
    raceFinished = false;
    playerWinner = false;

    // RACERS RESET
    auto& startGate = gates.at(0);
    auto& nextGate = gates.at(1);

    int index = 0;
    size_t totalEntities = mEntities.size();

    for (auto const& entity : mEntities) {
        auto& racer = gCoordinator.GetComponent<Racer>(entity);
        auto& racerTransf = gCoordinator.GetComponent<PhysxTransform>(entity);
        auto& racerVehicle = gCoordinator.GetComponent<VehicleComponent>(entity);
        racerVehicle.engineHeat = 0.0f;

        // 1. Reset gates, race completion and engulfment status
        racer.lastGate = &startGate;
        racer.targetGate = &nextGate;
        racer.raceCompletion = 0.0f;
        racer.engulfed = false;

        // 2. Compute Start position
        float spread = startGate.width * 0.5f;
        float laneSafetyFactor = 0.7f;
        float offsetMultiplier = (totalEntities > 1)
            ? ((float)index / (totalEntities - 1) - 0.5f) * laneSafetyFactor
            : 0.0f; //15%, 50% and 85%

        racer.targetPercLane = offsetMultiplier + 0.5f;
        logger::info("targetperlane: {0}", racer.targetPercLane);
        glm::vec3 startOffset = startGate.right * (offsetMultiplier * spread);
        racerTransf.pos = startGate.position + startOffset;

        // Vehicle a bit above ground
        racerTransf.pos.y += 0.5f;

        // 3. Orientation of vehicle
        glm::vec3 forward = glm::normalize(startGate.direction);
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(worldUp, forward));
        glm::vec3 up = glm::cross(forward, right);

        // column 0 : Right, column 1 : Up, column 2 : Forward
        glm::mat3 rotationMat(right, up, forward);

        // CONVERSION ET NORMALIZATION 
        racerTransf.rot = glm::normalize(glm::quat_cast(rotationMat));

        if (std::isnan(racerTransf.rot.w)) {
            logger::error("Orientation failed! Resetting to identity.");
            racerTransf.rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }

        // 2. PhysX synchronization
        if (gCoordinator.HasComponent<VehicleComponent>(entity)) {
            auto& vehicle = gCoordinator.GetComponent<VehicleComponent>(entity);
            if (!vehicle.instance) {
                index++;
                continue;
            }

            // Reset position, orientation and velocity
            physx::PxRigidActor* actor = vehicle.instance->getRigidActor();
            if (actor) {
                physx::PxTransform pxTrans(
                    physx::PxVec3(racerTransf.pos.x, racerTransf.pos.y, racerTransf.pos.z),
                    physx::PxQuat(racerTransf.rot.x, racerTransf.rot.y, racerTransf.rot.z, racerTransf.rot.w)
                );

                actor->setGlobalPose(pxTrans);

                physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
                if (dynamicActor) {

                    dynamicActor->setLinearVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));
                    dynamicActor->setAngularVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));
                    dynamicActor->clearForce();
                    dynamicActor->clearTorque();
                }
            }
        }
        index++;
    }

    this->refreshLeaderboard();

    // AVALANCHE RESET
    avalanche->gate = &gates.at(1);
    avalanche->gate->prevGate = &gates.at(0);
    glm::vec3 spawnPos = gates.at(0).position - (gates.at(0).direction * 150.0f);
    avalanche->mPosition = spawnPos;
    logger::warn("Avalanche reset with position: x {}, y {}, z {}", spawnPos.x, spawnPos.y, spawnPos.z);

    // Immediate Physic Reset
    if (avalanche->mPhysicsActor) {
        physx::PxTransform tp(physx::PxVec3(spawnPos.x, spawnPos.y, spawnPos.z),
            physx::PxQuat(0, 0, 0, 1));
        avalanche->mPhysicsActor->setGlobalPose(tp);
        avalanche->mPhysicsActor->setKinematicTarget(tp);
    }

    avalanche->mBaseSpeed = 5.0f;
    avalanche->mCloseProximityTimer = 0.0f;
    avalanche->raceCompletion = 0.0f;
    avalanche->mIsActive = true;

    logger::info("Race Restarted: {} vehicles on grid", leaderboard.size());
}

float RacingSystem::getDistanceToGateLine(const glm::vec3& racerPos, const Gate& gate) {
    // Flatten on XZ plane
    glm::vec3 racerPosFlat{ racerPos.x, 0.0f, racerPos.z };
    glm::vec3 gatePosFlat{ gate.position.x, 0.0f, gate.position.z };

    // Target vector: Racer to Gate Center
    glm::vec3 vecToTarget = gatePosFlat - racerPosFlat;

    // Right vector from Gate Center
    glm::vec3 gateRightFlat = glm::normalize(glm::vec3(gate.right.x, 0.0f, gate.right.z));

    // Projection of Target vector on Right vector
    glm::vec3 projLateral = glm::proj(vecToTarget, gateRightFlat);

    // Compute final distance vector: Racer to closest point on Gate Lane
    glm::vec3 vecToLine = vecToTarget - projLateral;

    return glm::length(vecToLine);
}

void RacingSystem::checkRacerEngulfment(Racer& racer, PhysxTransform& racerTransf)
{
    if (racer.engulfed) return;
    // Get the player position in world space
    glm::vec3 playerPos = racerTransf.pos;

    // Calculate the rotation quaternion from direction
    glm::vec3 defaultForward(0.f, 0.f, 1.f);
    glm::quat avalancheRotation = avalanche->mRotation;// = glm::rotation(defaultForward, avalanche->mDirection);

    // Transform player position into avalanche's local space
    // Translate to avalanche center
    glm::vec3 relativePos = racerTransf.pos - avalanche->mPosition;

    // Rotate by inverse of avalanche rotation to get local coordinates
    glm::quat invRotation = glm::inverse(avalancheRotation);
    glm::vec3 localPos = invRotation * relativePos;

    // Check if player is inside the oriented bounding box (in local space)
    glm::vec3 halfSize = avalanche->mSize / 2.0f;
    bool insideX = std::abs(localPos.x) < halfSize.x;
    bool insideY = std::abs(localPos.y) < halfSize.y;
    bool insideZ = std::abs(localPos.z) < halfSize.z;

    if (false && insideX && insideY && insideZ) {
        // Racer is engulfed!
        racer.engulfed = true;
        avalanche->mCloseProximityTimer = 0.0f;
        logger::warn("Racer at rank #{} engulfed by avalanche at position ({}, {}, {}) --> INSIDE avalanche",
            racer.currentRank, racerTransf.pos.x, racerTransf.pos.y, racerTransf.pos.z);
    }

    // Check if racer is behind the avalanche
    if (racer.raceCompletion < avalanche->raceCompletion) {
        racer.engulfed = true;
        logger::warn("Racer at rank #{} engulfed by avalanche at position ({}, {}, {}) --> BEHIND the avalanche",
            racer.currentRank, racerTransf.pos.x, racerTransf.pos.y, racerTransf.pos.z);
    }
}

void RacingSystem::init(std::shared_ptr<Avalanche> avalanchePtr) {
    this->avalanche = avalanchePtr;
    initGatesFromPoints();
    this->restart();
}

void RacingSystem::initGatesFromPoints() {
    constexpr glm::vec3 upVec{ 0.f, 1.f, 0.f };
    totalRaceLength = 0.0f;

    for (auto& gate : gates) {
        gate.position = (gate.leftPoint + gate.rightPoint) * 0.5f;

        gate.width = glm::distance(gate.leftPoint, gate.rightPoint);


        if (gate.width > 0.001f) {
            gate.right = (gate.rightPoint - gate.leftPoint) / gate.width;
            gate.forward = glm::normalize(glm::cross(gate.right, upVec));
            gate.up = glm::normalize(glm::cross(gate.right, gate.forward));
        }
    }

    for (size_t i = 0; i < gates.size(); i++) {
        auto& gate = gates.at(i);
        gate.raceLength = totalRaceLength;

        if (i == gates.size() - 1) {
            gate.direction = (i > 0) ? gates.at(i - 1).direction : glm::vec3(0, 0, 1);
            gate.nextGate = nullptr;
        }
        else {
            auto& nextGate = gates.at(i + 1);
            gate.nextGate = &nextGate;
            gate.lane = nextGate.position - gate.position;
            gate.laneLength = glm::length(gate.lane);
            totalRaceLength += gate.laneLength;
            gate.direction = glm::normalize(gate.lane);
        }

        if (i > 0) {
            gate.prevGate = &gates.at(i - 1);
        }

        gate.right = glm::normalize(glm::cross(gate.direction, upVec));

        std::string tex = (i == 0 || i == gates.size() - 1) ? "assets/textures/2k_mars.jpg" : "assets/textures/snowball.png";
        renderingSystem->createBoxEntity(tex, render::BoxConfig{
            gate.position,
            math::transform::quatFromDirection(gate.forward, gate.up),
            glm::vec3(gate.width, 0.1f, 0.1f)
        });
    }
}

/*
void RacingSystem::initGatesOld() {

    constexpr glm::vec3 upVec{ 0.f, 1.f, 0.f };

    for (size_t i = 0; i < gatesOld.size(); i++)
    {
        auto& gate = gatesOld.at(i);
        gate.raceLength = totalRaceLength;

        if (i == gatesOld.size() - 1) {
            gate.direction = gatesOld.at(i - 1).direction;
        }
        else {
            auto& nextGate = gatesOld.at(i + 1);
            gate.nextGate = &nextGate;
            gate.lane = nextGate.position - gate.position;
            gate.laneLength = glm::length(gate.lane);
            totalRaceLength += gate.laneLength;
            gate.direction = glm::normalize(gate.lane);
        }

        if (i > 0) {
            auto& prevGate = gatesOld.at(i - 1);
            gate.prevGate = &prevGate;
        }

        gate.right = glm::normalize(glm::cross(gate.direction, upVec));

        std::string tex = (i == 0 || i == gatesOld.size() - 1) ? "assets/textures/2k_mars.jpg" : "assets/textures/carbon_fiber.jpg";
        renderingSystem->createBoxEntity(tex, render::BoxConfig{
            gate.position,
            math::transform::quatFromDirection(gate.direction, upVec),
            glm::vec3(gate.width, 0.1f, 0.1f)
        });
    }
}
*/
