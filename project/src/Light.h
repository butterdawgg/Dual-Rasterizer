#pragma once

#include "Maths.h"

namespace dae
{
    struct Light
    {
        Vector3 direction { 0.0f, -1.0f, 0.0f };
        float padding1 { 0.0f }; // padding for 16-byte alignments
        ColorRGB color { colors::White };
        float intensity { 1.0f };

        explicit Light() = default;
        explicit Light(const Vector3& direction, const ColorRGB& color, float intensity);
    };
}