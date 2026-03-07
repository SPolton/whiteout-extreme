#pragma once
#include <PxPhysicsAPI.h>

struct RigidBody {
    physx::PxRigidActor* actor; // Use the base class pointer
    glm::vec3 linearVelocity = glm::vec3(0.0f); // store movement
};
