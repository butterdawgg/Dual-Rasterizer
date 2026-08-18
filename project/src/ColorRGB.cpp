#include "ColorRGB.h"
#include "Vector3.h"

namespace dae
{
    void ColorRGB::MaxToOne()
    {
        const float maxValue = std::max(r, std::max(g, b));
        if (maxValue > 1.f)
            *this /= maxValue;
    }

    ColorRGB ColorRGB::Lerp(const ColorRGB& c1, const ColorRGB& c2, float factor)
    {
        return { Lerpf(c1.r, c2.r, factor), Lerpf(c1.g, c2.g, factor), Lerpf(c1.b, c2.b, factor) };
    }

    const ColorRGB& ColorRGB::operator+=(const ColorRGB& c)
    {
        r += c.r;
        g += c.g;
        b += c.b;

        return *this;
    }

    ColorRGB ColorRGB::operator+(const ColorRGB& c) const
    {
        return { r + c.r, g + c.g, b + c.b };
    }

    const ColorRGB& ColorRGB::operator-=(const ColorRGB& c)
    {
        r -= c.r;
        g -= c.g;
        b -= c.b;

        return *this;
    }

    ColorRGB ColorRGB::operator-(const ColorRGB& c) const
    {
        return { r - c.r, g - c.g, b - c.b };
    }

    const ColorRGB& ColorRGB::operator*=(const ColorRGB& c)
    {
        r *= c.r;
        g *= c.g;
        b *= c.b;

        return *this;
    }

    ColorRGB ColorRGB::operator*(const ColorRGB& c) const
    {
        return { r * c.r, g * c.g, b * c.b };
    }

    const ColorRGB& ColorRGB::operator/=(const ColorRGB& c)
    {
        r /= c.r;
        g /= c.g;
        b /= c.b;

        return *this;
    }

    const ColorRGB ColorRGB::operator/(const ColorRGB& c) const
    {
        return { r / c.r, g / c.g, b / c.b };
    }

    const ColorRGB& ColorRGB::operator*=(float s)
    {
        r *= s;
        g *= s;
        b *= s;

        return *this;
    }

    ColorRGB ColorRGB::operator*(float s) const
    {
        return { r * s, g * s, b * s };
    }

    const ColorRGB& ColorRGB::operator/=(float s)
    {
        r /= s;
        g /= s;
        b /= s;

        return *this;
    }

    const ColorRGB ColorRGB::operator/(float s) const
    {
        return { r / s, g / s, b / s };
    }

    ColorRGB operator*(float s, const ColorRGB& c)
    {
        return c * s;
    }

    ColorRGB ToColorRGB(uint32_t pixel, SDL_PixelFormat* format)
    {
        Uint8 r { };
        Uint8 g { };
        Uint8 b { };

        SDL_GetRGB(pixel, format, &r, &g, &b);

        return ColorRGB {
            r / 255.f,
            g / 255.f,
            b / 255.f
        };
    }

    uint32_t ToUint32(const ColorRGB& color, SDL_PixelFormat* format)
    {
        const Uint8 r { static_cast<Uint8>(std::clamp(color.r * 255.0f, 0.0f, 255.0f)) };
        const Uint8 g { static_cast<Uint8>(std::clamp(color.g * 255.0f, 0.0f, 255.0f)) };
        const Uint8 b { static_cast<Uint8>(std::clamp(color.b * 255.0f, 0.0f, 255.0f)) };

        return SDL_MapRGB(format, r, g, b);
    }

    Vector3 ToVector3(const ColorRGB& color)
    {
        return Vector3 { color.r, color.g, color.b };
    }
}