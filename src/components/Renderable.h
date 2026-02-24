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
};
