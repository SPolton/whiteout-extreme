#pragma once

#include "PxPhysicsAPI.h"
#include "components/Entity.h"
#include "components/Transform.h"

class PhysicsObject {
public:
    PhysicsObject(const char* name);
    virtual ~PhysicsObject();

    // Pure virtual method to get the PhysX rigid actor
    virtual physx::PxRigidActor* getRigidActor() = 0;

    // Update the transform from the PhysX actor
    virtual void updateTransform();

    // Access to entity and transform
    EntityPx& getEntity() { return mEntity; }
    const EntityPx& getEntity() const { return mEntity; }
    PhysxTransform* getTransform() { return mTransform; }

protected:
    PhysxTransform* mTransform;
    EntityPx mEntity;
};
