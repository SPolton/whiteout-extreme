#pragma once

#include "core/buffer/Geometry.hpp"

namespace ShapeGenerator
{
    [[nodiscard]]
    CPU_Geometry sphere(float radius, int slices, int stacks);

    [[nodiscard]]
    CPU_Geometry unit_cube();

    [[nodiscard]]
    CPU_Geometry triangle_2D();

    [[nodiscard]]
    CPU_Geometry square_2D();
};
