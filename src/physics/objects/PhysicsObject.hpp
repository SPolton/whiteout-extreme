#pragma once

#include "components/Transform.h"

#include <PxRigidActor.h>
#include <memory>
#include <string>

class PhysicsObject {
public:
    PhysicsObject(const char* name);
    virtual ~PhysicsObject();

    // Pure virtual method to get the PhysX rigid actor
    virtual physx::PxRigidActor* getRigidActor() = 0;

    // Update the transform from the PhysX actor
    virtual void updateTransform();

    const std::string& name() const { return mName; }
    PhysxTransform* getTransform() { return mTransform.get(); }
    const PhysxTransform* getTransform() const { return mTransform.get(); }

protected:
    std::unique_ptr<PhysxTransform> mTransform;
    std::string mName;
};
