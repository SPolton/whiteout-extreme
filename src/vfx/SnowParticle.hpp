#pragma once

#include <glm/glm.hpp>

#include <vector>

struct SnowParticle {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 velocity{0.0f, 0.0f, 0.0f};
    float lifeSec = 0.0f;
    float size = 0.0f;
    glm::vec3 color{0.95f, 0.97f, 1.0f}; // default snow color

    bool isValid() const {
        return lifeSec > 0.0f && size > 0.0f;
    }
};

struct SnowFrame {
    std::vector<SnowParticle> particles;
};
