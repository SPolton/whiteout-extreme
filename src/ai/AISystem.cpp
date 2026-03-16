#include "AISystem.hpp"
#include "utils/logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <GLFW/glfw3.h>

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

    for (auto const& entity : mEntities) {
        auto& aiRacer = gCoordinator.GetComponent<Racer>(entity);
        auto& aiTransf = gCoordinator.GetComponent<PhysxTransform>(entity);
        auto& aiVehicle = gCoordinator.GetComponent<VehicleComponent>(entity);

        if (!aiRacer.targetGate) continue;

        // 1. Calculate direction vectors
        //glm::vec3 targetPos = aiRacer.getTargetPosition();
        glm::vec3 targetPos = aiRacer.getLookAheadTarget(10.f);
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
        aiVehicle.steer = glm::clamp(angle * steerDirection * 2.0f, -1.0f, 1.0f);


        // 4. Movement and Braking Logic
        float currentSpeed = aiVehicle.speed();
        float maxThrottle = 0.7f; 
        float angleDeg = glm::degrees(angle);

        if (aiVehicle.forwardGearDesired) {
            if (angleDeg > 60.f) {
                if (currentSpeed < 5.0f) { 
                    aiVehicle.throttle = maxThrottle;
                    aiVehicle.brake = 0.0f;
                }
                else {
                    aiVehicle.throttle = 0.2f;
                    aiVehicle.brake = 0.5f;
                }
            }
            else if (angleDeg > 20.f) {
                aiVehicle.brake = 0.0f;
                float throttleFactor = glm::clamp(1.0f - (angleDeg / 60.0f), 0.4f, 1.0f);
                aiVehicle.throttle = maxThrottle * throttleFactor;
            }
            else {
                aiVehicle.brake = 0.0f;
                aiVehicle.throttle = maxThrottle;
            }
        }

        // 5. Gear State Synchronization (Ensure PhysX gear matches intent)
        if (!aiVehicle.hasGearDesired()) {
            if (currentSpeed < 1.0f) {
                aiVehicle.setGearDesired();
            }
            else {
                // Must stop completely before shifting
                aiVehicle.throttle = 0.0f;
                aiVehicle.brake = 1.0f;
            }
        }

        if (shouldLog && false) {
            logger::info("AI {}: Gate {}, Angle: {:.2f}, Brake: {:.2f}, Throttle: {:.2f}, Forward: {}",
                entity, aiRacer.targetGate->id, angle, aiVehicle.brake, aiVehicle.throttle, aiVehicle.forwardGearDesired);
        }
    }
}
