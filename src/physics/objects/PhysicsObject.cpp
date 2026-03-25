#include "PhysicsObject.hpp"
#include "utils/logger.h"

using namespace physx;

PhysicsObject::PhysicsObject(const char* name)
    : mTransform(std::make_unique<PhysxTransform>())
    , mName(name ? name : "unnamed_entity")
{
}

PhysicsObject::~PhysicsObject() = default;

void PhysicsObject::updateTransform() {
    PxRigidActor* actor = getRigidActor();
    if (!actor) {
        logger::warn("PhysicsObject::updateTransform() called but getRigidActor() returned nullptr");
        return;
    }

    PxTransform pose = actor->getGlobalPose();

    // Update position
    mTransform->pos.x = pose.p.x;
    mTransform->pos.y = pose.p.y;
    mTransform->pos.z = pose.p.z;

    // Update rotation
    mTransform->rot.x = pose.q.x;
    mTransform->rot.y = pose.q.y;
    mTransform->rot.z = pose.q.z;
    mTransform->rot.w = pose.q.w;
}
