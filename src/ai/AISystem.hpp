#pragma once

#include "components/AI.h"
#include "components/Racer.h"
#include "components/VehicleComponent.h"
#include <memory>
#include "ecs/Coordinator.hpp"
#include "core/RenderingSystem.hpp"
#include "physics/PhysicsSystem.hpp"
#include "input/VehicleControlSystem.hpp"
#include "gameplay/SnowBallisticSystem.hpp"

class AISystem : public System {
public:
    AISystem(std::shared_ptr<RenderingSystem> renderingSystem,
        std::shared_ptr<PhysicsSystem> physicsSystem);

    void update(float deltaTime);

private:
    std::shared_ptr<RenderingSystem> renderingSystem;
    std::shared_ptr<PhysicsSystem> physicsSystem;

    float getDistSqToThrowAxis(const glm::vec3& p, const PhysxTransform& trans);

};
