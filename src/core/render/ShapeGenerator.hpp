#pragma once

#include "core/buffer/Geometry.hpp"

namespace ShapeGenerator
{
    [[nodiscard]]
    CPU_Geometry sphere(float radius, int slices, int stacks);

    [[nodiscard]]
    CPU_Geometry cube();

    [[nodiscard]]
    CPU_Geometry triangle();

    [[nodiscard]]
    CPU_Geometry square();

    [[nodiscard]]
    CPU_Geometry plane(float size);

    [[nodiscard]]
    CPU_Geometry infinitePlane(float size, float uvRepeat);
};
