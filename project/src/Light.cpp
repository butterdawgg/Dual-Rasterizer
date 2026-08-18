#include "Light.h"

namespace dae
{
    Light::Light(const Vector3& direction, const ColorRGB& color, float intensity) :
        direction(direction), color(color), intensity(intensity) { }
}