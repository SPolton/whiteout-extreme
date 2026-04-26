#pragma once
#include <PxPhysicsAPI.h>
#include <ext/vector_float3.hpp>

struct RigidBody {
    physx::PxRigidActor* actor = nullptr;  // Use the base class pointer
    glm::vec3 linearVelocity{};  // store movement
};
