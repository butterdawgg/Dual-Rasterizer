#pragma once

#include "MathHelpers.h"
#include "SDL_surface.h"

namespace dae
{
    struct Vector3;

    struct ColorRGB final
    {
        float r { };
        float g { };
        float b { };

        void MaxToOne();

        static ColorRGB Lerp(const ColorRGB& c1, const ColorRGB& c2, float factor);

        const ColorRGB& operator+=(const ColorRGB& c);

        ColorRGB operator+(const ColorRGB& c) const;

        const ColorRGB& operator-=(const ColorRGB& c);

        ColorRGB operator-(const ColorRGB& c) const;

        const ColorRGB& operator*=(const ColorRGB& c);

        ColorRGB operator*(const ColorRGB& c) const;

        const ColorRGB& operator/=(const ColorRGB& c);

        const ColorRGB operator/(const ColorRGB& c) const;

        const ColorRGB& operator*=(float s);

        ColorRGB operator*(float s) const;

        const ColorRGB& operator/=(float s);

        const ColorRGB operator/(float s) const;
    };

    ColorRGB operator*(float s, const ColorRGB& c);

    ColorRGB ToColorRGB(uint32_t pixel, SDL_PixelFormat* format);

    uint32_t ToUint32(const ColorRGB& color, SDL_PixelFormat* format);

    Vector3 ToVector3(const ColorRGB& color);

    namespace colors
    {
        static ColorRGB Red { 1, 0, 0 };
        static ColorRGB Blue { 0, 0, 1 };
        static ColorRGB Green { 0, 1, 0 };
        static ColorRGB Yellow { 1, 1, 0 };
        static ColorRGB Cyan { 0, 1, 1 };
        static ColorRGB Magenta { 1, 0, 1 };
        static ColorRGB White { 1, 1, 1 };
        static ColorRGB Black { 0, 0, 0 };
        static ColorRGB Gray { 0.5f, 0.5f, 0.5f };

        static ColorRGB ConrflowerBlue { 0.39f, 0.59f, 0.93f };
        static ColorRGB DarkGray { 0.1f, 0.1f, 0.1f };
        static ColorRGB LightGray { 0.39f, 0.39f, 0.39f };
    }
}