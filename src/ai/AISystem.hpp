#pragma once

#include "core/RenderingSystem.hpp"
#include "physics/PhysicsSystem.hpp"

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
