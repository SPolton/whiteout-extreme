#include "RacingSystem.hpp"
#include "utils/logger.h"
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

    for (auto const& entity : mEntities) {
        auto& racer = gCoordinator.GetComponent<Racer>(entity);
        auto& racerTransf = gCoordinator.GetComponent<PhysxTransform>(entity);

        if (!racer.targetGate || !racer.lastGate) continue;

        float distTarget = getDistanceToGateLine(racerTransf.pos, *racer.targetGate);
        float distLast = getDistanceToGateLine(racerTransf.pos, *racer.lastGate);

        float lengthOnLane = distLast / (distLast + distTarget) * racer.lastGate->laneLength;
        racer.raceCompletion = (racer.lastGate->raceLength + lengthOnLane) / totalRaceLength;

        if (shouldLog) {
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
            if (racer.targetGate->nextGate) {
                racer.lastGate = racer.targetGate;
                racer.targetGate = racer.targetGate->nextGate;
                logger::info("Next Gate !");
            }
        }
        // B. BACKWARD : checks if racer got back behind last gate
        else if (dotLast > 0.0f) {
            if (racer.lastGate->prevGate) {
                racer.targetGate = racer.lastGate;
                racer.lastGate = racer.lastGate->prevGate;
                logger::info("Back to Last Gate !");
            }
        }
    }
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

void RacingSystem::initGates() {

    constexpr glm::vec3 upVec{ 0.f, 1.f, 0.f };

    for (size_t i = 0; i < gates.size(); i++)
    {
        auto& gate = gates.at(i);
        gate.raceLength = totalRaceLength;

        if (i == gates.size()-1) {
            gate.direction = gates.at(i - 1).direction;
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
            auto& prevGate = gates.at(i - 1);
            gate.prevGate = &prevGate;
        }

        gate.right = glm::normalize(glm::cross(gate.direction, upVec));

        std::string tex = (i == 0 || i == gates.size()-1) ? "assets/textures/2k_mars.jpg" : "assets/textures/carbon_fiber.jpg";
        renderingSystem->createGateEntity(gate.position, gate.direction, gate.width, tex);
    }
}
