#include "MathHelpers.h"

namespace dae
{
    float Square(float a)
    {
        return a * a;
    }

    float Lerpf(float a, float b, float factor)
    {
        return ((1 - factor) * a) + (factor * b);
    }

    bool AreEqual(float a, float b, float epsilon)
    {
        return abs(a - b) < epsilon;
    }

    int Clamp(const int v, int min, int max)
    {
        if (v < min) return min;
        if (v > max) return max;
        return v;
    }

    float Clamp(const float v, float min, float max)
    {
        if (v < min) return min;
        if (v > max) return max;
        return v;
    }

    float Saturate(const float v)
    {
        if (v < 0.f) return 0.f;
        if (v > 1.f) return 1.f;
        return v;
    }
}