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

        if (aiVehicle.forwardGearDesired) {
            // Racing Forward:
            // If angle > 50°
            if (angle > glm::radians(50.f)) {
                // FLAG1: Sharp turn or orientation error

                if (currentSpeed < 1.5f) {
                    // DEBLOCKING LOGIC: We are too slow to turn properly.
                    // Force throttle to 100% of max and release brakes to "kick" the car 
                    // and allow the wheels to rotate the chassis.
                    aiVehicle.throttle = maxThrottle;
                    aiVehicle.brake = 0.0f;
                }
                else {
                    // Standard Braking: We have enough speed, so we slow down to tighten the radius
                    aiVehicle.throttle = 0.1f;
                    aiVehicle.brake = glm::clamp(angle * 0.6f, 0.3f, 0.8f);
                }
            }
            // If angle <= 50°
            else {
                // FLAG2: Straight line / Correct orientation
                aiVehicle.brake = 0.0f;
                float speedFactor = 1.0f - glm::clamp(angle, 0.0f, 0.5f);
                aiVehicle.throttle = maxThrottle * speedFactor;
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

        if (shouldLog) {
            logger::info("AI {}: Gate {}, Angle: {:.2f}, Brake: {:.2f}, Throttle: {:.2f}, Forward: {}",
                entity, aiRacer.targetGate->id, angle, aiVehicle.brake, aiVehicle.throttle, aiVehicle.forwardGearDesired);
        }
    }
}
