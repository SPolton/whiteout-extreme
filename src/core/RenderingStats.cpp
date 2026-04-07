#include "RenderingSystem.hpp"

#include <fmt/format.h>

using namespace render;
using Clock = std::chrono::steady_clock;
using Duration = std::chrono::duration<float, std::milli>;

std::string RenderingStats::toString() const
{
    if (!isEnabled) {
        return "Renderer profiling disabled";
    }

    return fmt::format(
        "Render frame: {:.1f} ms\n"
        "Geometry pass: {:.1f} ms\n"
        "Model pass: {:.1f} ms\n"
        "Particle pass: {:.1f} ms\n"
        "Renderable draws: {}\n"
        "Model draws: {}",
        frameMs, geometryMs,
        modelMs, particlesMs,
        renderableDrawCalls,
        modelDrawCalls
    );
}

void RenderingStats::startFrame()
{
    frameMs = 0.0f;
    geometryMs = 0.0f;
    modelMs = 0.0f;
    particlesMs = 0.0f;
    renderableDrawCalls = 0;
    modelDrawCalls = 0;
    geometryPassActive = false;
    modelPassActive = false;
    particlePassActive = false;

    if (!isEnabled) {
        return;
    }
}

void RenderingStats::startGeometryPass()
{
    if (!isEnabled || geometryPassActive) return;
    geometryStart = Clock::now();
    geometryPassActive = true;
}

void RenderingStats::endGeometryPass()
{
    if (!isEnabled || !geometryPassActive) return;
    geometryMs += Duration(Clock::now() - geometryStart).count();
    geometryPassActive = false;
}

void RenderingStats::startModelPass()
{
    if (!isEnabled || modelPassActive) return;

    modelStart = Clock::now();
    modelPassActive = true;
}

void RenderingStats::endModelPass()
{
    if (!isEnabled || !modelPassActive) return;
    modelMs += Duration(Clock::now() - modelStart).count();
    modelPassActive = false;
}

void RenderingStats::startParticlePass()
{
    if (!isEnabled || particlePassActive) return;
    particlesStart = Clock::now();
    particlePassActive = true;
}

void RenderingStats::endParticlePass()
{
    if (!isEnabled || !particlePassActive) return;
    particlesMs += Duration(Clock::now() - particlesStart).count();
    particlePassActive = false;
}

void RenderingStats::endFrame()
{
    if (!isEnabled) return;

    endGeometryPass();
    endModelPass();
    endParticlePass();

    frameMs = geometryMs + modelMs + particlesMs;
}
