#pragma once
#include <memory>
#include <glm/glm.hpp>

struct Gate {
    int id;

    glm::vec3 leftPoint;
    glm::vec3 rightPoint;

    float width;
    glm::vec3 position;

    glm::vec3 lane = glm::vec3(0.0f);
    float laneLength = 0.0f;
    float raceLength = 0.0f;

    glm::vec3 direction = glm::vec3(0.0f);
    glm::vec3 right = glm::vec3(0.0f);

    Gate* nextGate = nullptr;
    Gate* prevGate = nullptr;
};

struct Racer {
    const Gate* lastGate = nullptr;
    const Gate* targetGate = nullptr;
    float targetPercLane = 0.5f;
    float raceCompletion = 0.0f;
    int currentRank = 0;

    glm::vec3 getTargetPosition() const
    {
        if (!targetGate) return glm::vec3(0.0f);
        glm::vec3 offset = targetGate->right * ((targetPercLane - 0.5f) * targetGate->width);
        return targetGate->position + offset;
    };
};
