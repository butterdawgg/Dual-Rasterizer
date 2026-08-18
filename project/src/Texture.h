#pragma once

#include <SDL_surface.h>
#include <string>

#include "ColorRGB.h"
#include "DXIncludes.h"

namespace dae
{
    struct Vector2;

    class Texture final
    {
        public:

        ~Texture();

        static Texture* LoadFromFile(const std::string& path);

        ColorRGB Sample(float u, float v) const;
        ColorRGB Sample(const Vector2& uv) const;

        ID3D11ShaderResourceView* CreateShaderResourceView(ID3D11Device* pDevice);

        ID3D11ShaderResourceView* GetShaderResourceView() const;

        private:

        Texture(SDL_Surface* pSurface);

        SDL_Surface* m_pSurface { nullptr };
        uint32_t* m_pSurfacePixels { nullptr };

        // DirectX resource
        ID3D11ShaderResourceView* m_pShaderResourceView { nullptr };
    };
}