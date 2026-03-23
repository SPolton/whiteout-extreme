#pragma once

#include "vfx/SnowParticle.hpp"

#include <glm/glm.hpp>

class SnowRenderer {
public:
    SnowRenderer() = default;

    void beginFrame();
    void submitSnowFrame(const SnowFrame& frame);
    void render(const glm::mat4& view, const glm::mat4& projection);

    std::size_t submittedParticleCount() const;

private:
    SnowFrame currentPacket;
};
