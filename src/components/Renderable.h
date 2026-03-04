#pragma once

#include "core/assets/Texture.hpp"
#include "core/buffer/Geometry.hpp"
#include "core/render/ShaderProgram.hpp"
#include <memory>

struct Renderable {
    std::shared_ptr<GPU_Geometry> geometry;
    std::shared_ptr<CPU_Geometry> cpuData;
    std::shared_ptr<ShaderProgram> shader;
    std::shared_ptr<Texture> texture;
    bool isSkybox = false;

    // For rolling texture effect
    bool hasRollingTexture = false;
    float scrollScale = 0.1f; // Adjust this to control scroll speed
    glm::vec2 textureScrollOffset{};

    // Update rolling texture based on position
    void updateRollingTexture(glm::vec3 position) {
        if (!hasRollingTexture) return;

        position = position * scrollScale;

        textureScrollOffset.x = position.x * scrollScale;
        textureScrollOffset.y = position.z * scrollScale;
    }
};
