#pragma once

#include <glm/glm.hpp>

class SnowRenderer {
public:
    SnowRenderer() = default;

    void beginFrame();
    void render(const glm::mat4& view, const glm::mat4& projection);
};
