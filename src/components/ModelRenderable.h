#pragma once

#include "core/assets/ModelLoader.hpp"
#include "core/render/ShaderProgram.hpp"
#include <memory>

// Component for entities that render complex 3D models (loaded from OBJ and FBX)
// Unlike the simple Renderable component which uses pre-generated geometry,
// this component stores a reference to a loaded Model with multiple meshes and textures
struct ModelRenderable {
    ModelLoader* modelLoader;  // Non-owning pointer to the model loader
    ShaderProgram* shader;     // Non-owning pointer to shader
};
