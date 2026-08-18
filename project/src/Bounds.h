#pragma once

#include <vector>
#include "Maths.h"

namespace dae
{
    struct Vertex;

    struct Bounds final
    {
        Vector3 min { };
        Vector3 max { };

        explicit Bounds() = default;
        explicit Bounds(const Vector3& min, const Vector3& max);

        /* Fits the bounds to the vertices */
        void Fit(std::vector<Vertex>& vertices);
    };
}