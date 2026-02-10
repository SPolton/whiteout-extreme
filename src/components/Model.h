// Model.h
#pragma once
#include <glm.hpp>
#include <vector>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 col;
};

// Simple model representation for basic geometry
// For complex 3D models loaded from files, use core/assets/Model.hpp instead
class SimpleModel {
public:
    std::vector<Vertex> verts;
    glm::mat4 modelMatrix;
};
