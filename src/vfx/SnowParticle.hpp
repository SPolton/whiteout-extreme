#pragma once

#include <glm/glm.hpp>

#include <vector>

struct SnowParticle {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 velocity{0.0f, 0.0f, 0.0f};
    float lifeSec = 0.0f;
    float size = 0.0f;
};

struct SnowFrame {
    std::vector<SnowParticle> particles;
};
