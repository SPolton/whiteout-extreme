// Model.h
#pragma once

#include "core/assets/ModelLoader.hpp"
#include "core/render/ShaderProgram.hpp"
#include <glm.hpp>
#include <memory>
#include <vector>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 col;
};

// Simple model representation for basic geometry
class SimpleModel {
public:
    std::vector<Vertex> verts;
    glm::mat4 modelMatrix;
};

// Component for entities that render complex 3D models (loaded from OBJ and FBX)
// Unlike the simple Renderable component which uses pre-generated geometry,
// this component owns a loaded Model with multiple meshes and textures
struct ModelRenderable {
    std::shared_ptr<ModelLoader> modelLoader;  // Shared ownership
    std::shared_ptr<ShaderProgram> shader;     // Shared ownership
};
