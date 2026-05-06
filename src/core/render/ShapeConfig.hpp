#pragma once

#include "core/gl/GLHandles.hpp"
#include <ext/vector_float3.hpp>
#include <glm/gtx/quaternion.hpp>

namespace render
{

struct ShapeConfig {
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::quat rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
    GLenum textureWrapMode = GL_CLAMP_TO_EDGE;
    GLenum textureFilterMode = GL_LINEAR;
};

struct BoxConfig : public ShapeConfig {
    // Additional box-specific configuration options can be added here
};

// Configuration structs for entity creation
struct SphereConfig : public ShapeConfig {
    float radius = 1.0f;
    int slices = 16;
    int stacks = 16;
    bool isSkybox = false;
};

struct PlaneConfig : public ShapeConfig {
    float size = 10.0f;
    float uvRepeat = 1.0f;  // Set to > 1.0 for tiling effect
    bool isInfinite = false;  // If true, uses infinitePlaneSize with repeating UVs
    float infinitePlaneSize = 10000.0f;  // Size for infinite planes
};

struct ModelConfig : public ShapeConfig {
    // Additional model-specific configuration options can be added here
};

struct GateConfig : public ShapeConfig {
    glm::vec3 direction = { 0.0f, 0.0f, 1.0f };
    float width = 1.0f;
};

} // namespace render
