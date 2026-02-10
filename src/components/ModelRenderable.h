#pragma once

#include "core/assets/ModelLoader.hpp"
#include "core/render/ShaderProgram.hpp"
#include <memory>

// Component for entities that render complex 3D models (loaded from OBJ and FBX)
// Unlike the simple Renderable component which uses pre-generated geometry,
// this component owns a loaded Model with multiple meshes and textures
struct ModelRenderable {
    std::shared_ptr<ModelLoader> modelLoader;  // Shared ownership
    ShaderProgram* shader;                      // Non-owning pointer to shader
};
