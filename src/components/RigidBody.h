#pragma once
#include <PxPhysicsAPI.h>

struct RigidBody {
    physx::PxRigidActor* actor; // Use the base class pointer
};
