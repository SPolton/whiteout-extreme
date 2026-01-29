#pragma once

#include "core/buffer/Geometry.h"

namespace ShapeGenerator
{
    [[nodiscard]]
    CPU_Geometry Sphere(float radius, int slices, int stacks);

    CPU_Geometry UnitCube();
};
