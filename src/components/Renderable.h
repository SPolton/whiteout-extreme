#pragma once

#include "../core/buffer/Geometry.hpp"
#include "../core/render/ShaderProgram.hpp"
#include "../core/assets/Texture.hpp"

struct Renderable {
    GPU_Geometry* geometry;
    CPU_Geometry* cpuData;
    ShaderProgram* shader;
    Texture* texture;
};
