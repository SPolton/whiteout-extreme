#pragma once

#include "core/assets/ModelLoader.hpp"
#include "core/assets/Texture.hpp"
#include "core/buffer/Geometry.hpp"
#include "core/render/ShaderProgram.hpp"
#include <memory>

// Shared material
struct RenderMaterial {
    std::shared_ptr<ShaderProgram> shader{};
    std::shared_ptr<Texture> baseTexture{};
    glm::vec2 textureScrollOffset{};
    bool useTextureScroll = false;
    bool useModelLighting = false;
};

struct Renderable {
    std::shared_ptr<GPU_Geometry> geometry{};
    std::shared_ptr<CPU_Geometry> cpuData{};
    RenderMaterial material{};
    bool isSkybox = false;

    // For rolling texture effect
    float scrollScale = 0.1f; // Adjust this to control scroll speed

    // Update rolling texture based on position
    // Only calculates offset if material.useTextureScroll is enabled
    void updateRollingTexture(glm::vec3 position) {
        if (!material.useTextureScroll) return;

        position = position * scrollScale;

        material.textureScrollOffset.x = position.x * scrollScale;
        material.textureScrollOffset.y = position.z * scrollScale;
    }
};

// Component for entities that render complex 3D models (loaded from OBJ and FBX)
// Unlike the simple Renderable component which uses pre-generated geometry,
// this component owns a loaded Model with multiple meshes and textures
struct ModelRenderable {
    std::shared_ptr<ModelLoader> modelLoader{};  // Shared ownership
    RenderMaterial material{};

    glm::vec3 visualOffsetPos = glm::vec3(0.0f);
};
