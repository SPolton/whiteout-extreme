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
    bool shouldLog = false;

    if (logTimer >= 1.f) {
        shouldLog = true;
        logTimer = 0.0f;
    }

    for (auto const& entity : mEntities) {
        auto& aiRacer = gCoordinator.GetComponent<Racer>(entity);
        auto& aiLogic = gCoordinator.GetComponent<AI>(entity);
        auto& aiTransf = gCoordinator.GetComponent<PhysxTransform>(entity);
        auto& aiVehicle = gCoordinator.GetComponent<VehicleComponent>(entity);

        if (!aiRacer.targetGate) continue;

        // 1. Calculate direction vectors (3D space)
        glm::vec3 targetPos = aiRacer.getTargetPosition();

        // Calculate vector from AI to target and normalize it
        glm::vec3 toTargetVec = targetPos - aiTransf.pos;
        float distanceToTarget = glm::length(toTargetVec);

        // Prevent normalization of a zero vector if the AI is exactly on the target
        glm::vec3 toTarget = (distanceToTarget > 0.001f) ? glm::normalize(toTargetVec) : aiTransf.getForwardVector();

        // Get the current facing direction of the vehicle
        glm::vec3 forward = aiTransf.getForwardVector();

        // 2. Calculate angle between Forward and Target (Dot Product)
        // Clamp the dot product to [-1, 1] to prevent NaN errors in acos due to floating point precision
        float dot = glm::clamp(glm::dot(forward, toTarget), -1.0f, 1.0f);
        float angle = glm::acos(dot);

        // 3. Determine steering direction (Cross Product)
        // The sign of the Y component tells us if the target is to the left or right of the forward vector
        glm::vec3 crossResult = glm::cross(forward, toTarget);
        float steerDirection = (crossResult.y > 0.0f) ? 1.0f : -1.0f;

        // 4. Calculate Final Steering (Error * Direction)
        // Small angles result in subtle steering; large angles result in sharp turns
        float steerError = angle * steerDirection;

        // Apply a gain factor (e.g., 1.5f) to increase steering responsiveness
        aiVehicle.steer = glm::clamp(steerError * 1.5f, -1.0f, 1.0f);

        // 5. Speed Management (Throttle / Brake)
        // Reduce speed proportionally to the sharpness of the turn (steering angle)
        // A speedFactor of 1.0 means full cruise speed; lower means slowing down for curves
        float speedFactor = 1.0f - glm::clamp(glm::abs(steerError) / glm::pi<float>(), 0.0f, 0.7f);
        aiVehicle.throttle = 0.7f * speedFactor; // Base cruise speed scaled by the curve factor

        // Apply braking if the turn angle is too sharp (roughly > 45 degrees)
        aiVehicle.brake = (glm::abs(steerError) > 0.8f) ? 0.2f : 0.0f;

        if (shouldLog) {
            logger::info("AI Entity {}: Target Gate {}, Steer: {:.2f}, Throttle: {:.2f}",
                entity, aiRacer.targetGate->id, aiVehicle.steer, aiVehicle.throttle);
        }
    }
}
