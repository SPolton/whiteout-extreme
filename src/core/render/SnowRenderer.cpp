#include "SnowRenderer.hpp"

void SnowRenderer::beginFrame()
{
}

void SnowRenderer::submitSnowFrame(const SnowFrame& frame)
{
    currentPacket = frame;
}

void SnowRenderer::render(const glm::mat4& view, const glm::mat4& projection)
{
    (void)view;
    (void)projection;
}

std::size_t SnowRenderer::submittedParticleCount() const
{
    return currentPacket.particles.size();
}
