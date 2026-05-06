#include "AISystem.hpp"

#include "components/Racer.h"
#include "components/VehicleComponent.h"
#include "gameplay/SnowBallisticSystem.hpp"
#include "input/VehicleControlSystem.hpp"
#include "utils/logger.h"

#include <glm/gtc/constants.hpp>

extern Coordinator gCoordinator;

AISystem::AISystem(
    std::shared_ptr<RenderingSystem> renderingSystem,
    std::shared_ptr<PhysicsSystem> physicsSystem)
    : renderingSystem(renderingSystem),
    physicsSystem(physicsSystem)
{
}

void AISystem::update(float deltaTime)
{
    static float logTimer = 0.0f;
    logTimer += deltaTime;
    bool shouldLog = (logTimer >= 4.f);
    if (shouldLog) logTimer = 0.0f;

    const auto& vehicles = gCoordinator.GetSystem<VehicleControlSystem>()->mEntities;

    for (auto const& entity : mEntities) {
        auto& aiRacer = gCoordinator.GetComponent<Racer>(entity);
        auto& aiTransf = gCoordinator.GetComponent<PhysxTransform>(entity);
        auto& aiVehicleComponent = gCoordinator.GetComponent<VehicleComponent>(entity);
        if (!aiVehicleComponent.instance) {
            continue;
        }
        auto& aiVehicle = *aiVehicleComponent.instance;

        /// --- Handling SnowBall Throw Logic ---
        if (aiVehicleComponent.snowBallCooldown <= 0.0f) {
            for (auto const& vehicle : vehicles) {
                if (entity == vehicle) continue;
                auto& vTrans = gCoordinator.GetComponent<PhysxTransform>(vehicle);
                glm::vec3 relPos = vTrans.pos - aiTransf.pos;

                // 1. Quick sphere check (40m^2 = 1600)
                float d2 = glm::dot(relPos, relPos);
                if (d2 > 3600.f) continue;

                // 1. Front/Back check (Dot product)
                if (glm::dot(relPos, aiTransf.forward()) < 0) continue;

                // 2. Lateral stream check (10m^2 = 100)
                if (getDistSqToThrowAxis(vTrans.pos, aiTransf) < 16.f) {
                    gCoordinator.GetSystem<SnowBallisticSystem>()->throwSnowball(entity);
                    aiVehicleComponent.isBoosting = false;
                    break;
                }
            }
        }
        
        /// --- Handling Boost System with Risk Factor ---

        // 1. Default safety threshold (AI normally stops at 95%)
        float safetyThreshold = 0.95f;

        // 2. Introduce a "Rare Mistake" chance
        // We check this every frame, so we use a very small probability (e.g., 2 in 1000)
        // This simulates a momentary lapse in judgment or "tunnel vision"
        if (rand() % 1000 < 900) {
            // AI becomes reckless and won't stop boosting until it's too late
            safetyThreshold = 1.2f;
        }

        // 3. Apply Boost Logic
        if (aiVehicleComponent.engineHeat < 0.1f || (aiVehicleComponent.engineHeat <= safetyThreshold && aiVehicleComponent.isBoosting)) {
            aiVehicleComponent.isBoosting = true;

            // Apply instant heat cost if enough time has passed since the last burst
            if (aiVehicleComponent.timeSinceLastBoost > 0.1f) {
                aiVehicleComponent.engineHeat = std::min(1.0f, aiVehicleComponent.engineHeat + aiVehicleComponent.boostHeatInstantCost());
            }
        }
        else {
            // Stop boosting if threshold reached or safety triggered
            aiVehicleComponent.isBoosting = false;
        }
        if (!aiRacer.targetGate) continue;

        /// --- Handling Movement and Commands ---
        // 1. Calculate direction vectors
        //glm::vec3 targetPos = aiRacer.getTargetPosition();
        glm::vec3 targetPos = aiRacer.getLookAheadTarget(20.f);
        glm::vec3 toTargetVec = targetPos - aiTransf.pos;
        float distanceToTarget = glm::length(toTargetVec);
        glm::vec3 toTarget = (distanceToTarget > 0.001f) ? glm::normalize(toTargetVec) : aiTransf.getForwardVector();
        glm::vec3 forward = aiTransf.getForwardVector();

        // 2. Calculate angle between forward vector and target direction
        float dot = glm::clamp(glm::dot(forward, toTarget), -1.0f, 1.0f);
        float angle = glm::acos(dot); // Radians [0, PI]

        // 3. Steering direction (Left or Right)
        glm::vec3 crossResult = glm::cross(forward, toTarget);
        float steerDirection = (crossResult.y > 0.0f) ? 1.0f : -1.0f;
        float steer = glm::clamp(angle * steerDirection * 2.0f, -1.0f, 1.0f);

        aiVehicleComponent.visualSteer = steer;

        // 4. Movement and Braking Logic
        float currentSpeed = aiVehicle.speed();
        float maxThrottle = aiVehicleComponent.isBoosting ? 1.0f : 0.7f;
        float angleDeg = glm::degrees(angle);
        float throttle = 0.0f;
        float brake = 0.0f;
        bool forwardGearDesired = aiVehicle.forwardGearDesired();

        if (forwardGearDesired) {
            if (angleDeg > 60.f) {
                if (currentSpeed < 5.0f) { 
                    throttle = maxThrottle;
                    brake = 0.0f;
                }
                else {
                    throttle = 0.2f;
                    brake = 0.5f;
                }
            }
            else if (angleDeg > 20.f) {
                float throttleFactor = glm::clamp(1.0f - (angleDeg / 60.0f), 0.4f, 1.0f);
                throttle = maxThrottle * throttleFactor;
                brake = 0.0f;
            }
            else {
                throttle = maxThrottle;
                brake = 0.0f;
            }
        }

        aiVehicle.applyDriveCommand(throttle, brake, steer, forwardGearDesired);

        if (shouldLog && false) {
            logger::info("AI {}: Gate {}, Angle: {:.2f}, Brake: {:.2f}, Throttle: {:.2f}, Forward: {}",
                entity, aiRacer.targetGate->id, angle, brake, throttle, forwardGearDesired);
        }        
    }
}

float AISystem::getDistSqToThrowAxis(const glm::vec3& p, const PhysxTransform& trans) {
    glm::vec3 dir = trans.forward();
    glm::vec3 v = p - trans.pos;
    float projection = glm::dot(v, dir);
    // Projection distance along the throw line axis
    glm::vec3 closestPoint = trans.pos + (dir * projection);
    glm::vec3 distVec = p - closestPoint;
    return glm::dot(distVec, distVec);
}
