#pragma once

#include "core/assets/Texture.hpp"
#include "core/buffer/Geometry.hpp"
#include "core/render/ShaderProgram.hpp"

struct Renderable {
    GPU_Geometry* geometry;
    CPU_Geometry* cpuData;
    ShaderProgram* shader;
    Texture* texture;
};
