#include <PxActor.h>
#include <PxSimulationEventCallback.h>

#include "audio/AudioEngine.h"
#include "input/glfw/InputManager.hpp"
#include "utils/logger.h"

class ContactReportCallback : public physx::PxSimulationEventCallback {
public:
    ContactReportCallback(
        std::shared_ptr<InputManager> inputManager,
        std::shared_ptr<AudioEngine> audioManager,
        float* gameTime) :
            audioManager(audioManager),
            inputManager(inputManager),
            gameTime(gameTime) {}

    void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
    {
        PX_UNUSED(pairHeader);
        PX_UNUSED(pairs);
        PX_UNUSED(nbPairs);

        // get name of colliding vehicles
        std::string v1Name = pairHeader.actors[0]->getName() ? pairHeader.actors[0]->getName() : "Unnamed";
        std::string v2Name = pairHeader.actors[1]->getName() ? pairHeader.actors[1]->getName() : "Unnamed";

        logger::debug("Contact reported between actors {} and {}", v1Name, v2Name);

        // get current game time
        float currentTime = *gameTime;

        // only play crash sound if not already recently played
        if ((currentTime - lastCrashTime) > crashCooldown) {
            // if contact with player vehicle
            if (v1Name == "VehiclePlayer1" || v2Name == "VehiclePlayer1") {
                // play crash sound on vehicle-to-vehicle contact
                audioManager->jsonSound("physics.crash");

                // send vibration feedback for crash
                inputManager->rumble(1.0f);

                // update time
                lastCrashTime = currentTime;
            }
        }
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

private:
    // audio and input pointer
    std::shared_ptr<AudioEngine> audioManager;
    std::shared_ptr<InputManager> inputManager;

    float* gameTime = nullptr;

    // cooldown for crash sound
    float lastCrashTime = 0.0f;
    float crashCooldown = 0.5f;
};
