#include "SnowRenderer.hpp"

#include "core/render/ShaderProgram.hpp"

#include <glad/glad.h>

SnowRenderer::SnowRenderer()
    : vao(), vbo()
{
}

void SnowRenderer::beginFrame()
{
    if (!initialized) {
        initialize();
    }
}

void SnowRenderer::submitSnowFrame(const SnowFrame& frame)
{
    currentSnowFrame = frame;
}

void SnowRenderer::render(const glm::mat4& view, const glm::mat4& projection)
{
    if (!initialized || !shader || currentSnowFrame.particles.empty()) {
        return;
    }

    uploadFrame();

    shader->use();

    glUniformMatrix4fv(
        glGetUniformLocation(*shader, "view"),
        1,
        GL_FALSE,
        &view[0][0]
    );
    glUniformMatrix4fv(
        glGetUniformLocation(*shader, "projection"),
        1,
        GL_FALSE,
        &projection[0][0]
    );

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(gpuVertices.size()));
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

std::size_t SnowRenderer::submittedParticleCount() const
{
    return currentSnowFrame.particles.size();
}

void SnowRenderer::initialize()
{
    shader = assetManager.loadShader("snow_particle");

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SnowGpuVertex), (void*)offsetof(SnowGpuVertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(SnowGpuVertex), (void*)offsetof(SnowGpuVertex, size));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(SnowGpuVertex), (void*)offsetof(SnowGpuVertex, lifeSec));

    glBindVertexArray(0);

    initialized = true;
}

void SnowRenderer::uploadFrame()
{
    gpuVertices.clear();
    gpuVertices.reserve(currentSnowFrame.particles.size());

    for (const SnowParticle& particle : currentSnowFrame.particles) {
        gpuVertices.push_back(SnowGpuVertex{
            .position = particle.position,
            .size = particle.size * 14.0f,
            .lifeSec = particle.lifeSec
        });
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(gpuVertices.size() * sizeof(SnowGpuVertex)),
        gpuVertices.data(),
        GL_STREAM_DRAW
    );
}
