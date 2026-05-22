#pragma once
#include <memory>
#include <glm/glm.hpp>

struct Gate {
    int id = 0;

    glm::vec3 leftPoint = glm::vec3(0.0f);
    glm::vec3 rightPoint = glm::vec3(0.0f);

    float width = 0.0f;
    glm::vec3 position = glm::vec3(0.0f);

    glm::vec3 lane = glm::vec3(0.0f);
    float laneLength = 0.0f;
    float raceLength = 0.0f;

    glm::vec3 direction = glm::vec3(0.0f);
    glm::vec3 right = glm::vec3(0.0f);
    glm::vec3 forward = glm::vec3(0.0f);
    glm::vec3 up = glm::vec3(0.0f);

    Gate* nextGate = nullptr;
    Gate* prevGate = nullptr;

    glm::vec3 getPositionAtPercentage(float percentage) const
    {
        glm::vec3 offset = right * ((percentage - 0.5f) * width);
        return position + offset;
    };

    glm::vec3 getLaneAtPercentage(float percentage) {
        return glm::vec3(nextGate->getPositionAtPercentage(percentage) - getPositionAtPercentage(percentage));
    }
};

struct Racer {
    const Gate* lastGate = nullptr;
    const Gate* targetGate = nullptr;
    float targetPercLane = 0.5f;
    float lengthOnLane = 0.0f;
    float raceCompletion = 0.0f;
    int currentRank = 0;
    bool engulfed = false;

    glm::vec3 getTargetPosition() const
    {
        if (!targetGate) return glm::vec3(0.0f);
        glm::vec3 offset = targetGate->right * ((targetPercLane - 0.5f) * targetGate->width);
        return targetGate->position + offset;
    };

    glm::vec3 getLookAheadTarget(float lookAheadDistance)
    {
        if (!targetGate || !lastGate) return (lastGate) ? lastGate->position : glm::vec3(0.0f);

        // 1. Start from current pos on lane
        const Gate* currentIter = lastGate;
        float distToOffset = lengthOnLane + lookAheadDistance;

        // 2. We go through gates to find correct lane segment
        while (currentIter->nextGate && distToOffset > currentIter->laneLength) {
            distToOffset -= currentIter->laneLength;
            currentIter = currentIter->nextGate;
        }

        // 3. Compute progression ratio in the correct lane segment
        float t = 0.0f;
        if (currentIter->laneLength > 0.0001f) {
            t = distToOffset / currentIter->laneLength;
        }
        t = glm::clamp(t, 0.0f, 1.0f);

        // 4. Retrieve the lane at the correct percentage between the 2 gates
        glm::vec3 laneStart = currentIter->getPositionAtPercentage(targetPercLane);
    
        if (currentIter->nextGate) {
            glm::vec3 laneEnd = currentIter->nextGate->getPositionAtPercentage(targetPercLane);
            // Interpolation to find lookAhead target on this lane
            return glm::mix(laneStart, laneEnd, t);
        }

        // At end of race:
        return getTargetPosition();
    }
};
