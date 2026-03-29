#pragma once

#include "core/assets/AssetManager.hpp"
#include "core/gl/GLHandles.hpp"
#include "vfx/SnowParticle.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class ShaderProgram;

struct SnowGpuVertex {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    float size = 1.0f;
    float lifeSec = 1.0f;
};

class SnowRenderer {
public:
    SnowRenderer();

    void beginFrame();
    void submitSnowFrame(const SnowFrame& frame);
    void render(const glm::mat4& view, const glm::mat4& projection);

    std::size_t submittedParticleCount() const;

private:
    AssetManager& assetManager = AssetManager::getInstance();

    SnowFrame currentSnowFrame;
    std::vector<SnowGpuVertex> gpuVertices;

    std::shared_ptr<ShaderProgram> shader;
    VertexArrayHandle vao;
    VertexBufferHandle vbo;

    bool initialized = false;

    void initialize();
    void uploadFrame();
};
