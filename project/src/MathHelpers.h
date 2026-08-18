#pragma once

#include <cmath>
#include <cfloat>
#include <algorithm>

namespace dae
{
    struct Int2
    {
        int x { };
        int y { };
    };

    constexpr auto PI = 3.14159265358979323846f;
    constexpr auto PI_DIV_2 = 1.57079632679489661923f;
    constexpr auto PI_DIV_4 = 0.785398163397448309616f;
    constexpr auto PI_2 = 6.283185307179586476925f;
    constexpr auto PI_4 = 12.56637061435917295385f;
    constexpr auto INV_PI = 0.31830988618f;

    constexpr auto TO_DEGREES = (180.0f / PI);
    constexpr auto TO_RADIANS(PI / 180.0f);

    float Square(float a);

    float Lerpf(float a, float b, float factor);

    bool AreEqual(float a, float b, float epsilon = FLT_EPSILON);

    int Clamp(const int v, int min, int max);

    float Clamp(const float v, float min, float max);

    float Saturate(const float v);
}