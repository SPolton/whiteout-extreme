#include <PxSimulationEventCallback.h>
#include "utils/logger.h"

class ContactReportCallback : public physx::PxSimulationEventCallback {
    void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
    {
        PX_UNUSED(pairHeader);
        PX_UNUSED(pairs);
        PX_UNUSED(nbPairs);

        logger::debug("Contact reported between actors {} and {}", 
            pairHeader.actors[0]->getName() ? pairHeader.actors[0]->getName() : "Unnamed",
            pairHeader.actors[1]->getName() ? pairHeader.actors[1]->getName() : "Unnamed"
        );
    }
    void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {
        PX_UNUSED(constraints);
        logger::trace("Constraint break reported, count: {}", count);
    }
    void onWake(physx::PxActor** actors, physx::PxU32 count) {
        PX_UNUSED(actors);
        logger::trace("Actor wake reported, count: {}", count);
    }
    void onSleep(physx::PxActor** actors, physx::PxU32 count) {
        PX_UNUSED(actors);
        logger::trace("Actor sleep reported, count: {}", count);
    }
    void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {
        PX_UNUSED(pairs);
        logger::trace("Trigger reported, count: {}", count);
    }
    void onAdvance(const physx::PxRigidBody* const* bodyBuffer,
        const physx::PxTransform* poseBuffer,
        const physx::PxU32 count) {
        PX_UNUSED(bodyBuffer);
        PX_UNUSED(poseBuffer);
        logger::trace("Advance reported, count: {}", count);
    }
};
