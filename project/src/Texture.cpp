#include "Texture.h"
#include "Vector2.h"

#include <SDL_image.h>
#include <iostream>

namespace dae
{
    Texture::Texture(SDL_Surface* pSurface) :
        m_pSurface { pSurface },
        m_pSurfacePixels { (uint32_t*)pSurface->pixels }
    { }

    Texture::~Texture()
    {
        if (m_pShaderResourceView)
        {
            m_pShaderResourceView->Release();
            m_pShaderResourceView = nullptr;
        }

        if (m_pSurface)
        {
            SDL_FreeSurface(m_pSurface);
            m_pSurface = nullptr;
        }
    }

    Texture* Texture::LoadFromFile(const std::string& path)
    {
        SDL_Surface* pSurface { IMG_Load(path.c_str()) };

        if (pSurface == nullptr)
        {
            std::cerr << "Failed to load texture: " << path
                << " | IMG_Error: " << IMG_GetError() << "\n";

            return nullptr;
        }

        // Ensure RGBA32 format
        SDL_Surface* pConvertedSurface {
            SDL_ConvertSurfaceFormat(pSurface, SDL_PIXELFORMAT_RGBA32, 0) };

        SDL_FreeSurface(pSurface);

        if (!pConvertedSurface)
        {
            std::cerr << "Failed to convert surface to RGBA32\n";

            return nullptr;
        }

        return new Texture { pConvertedSurface };
    }

    ColorRGB Texture::Sample(const Vector2& uv) const
    {
        return Sample(uv.x, uv.y);
    }

    ID3D11ShaderResourceView* Texture::CreateShaderResourceView(ID3D11Device* pDevice)
    {
        if (!pDevice || !m_pSurface)
            return nullptr;

        // Create texture description
        D3D11_TEXTURE2D_DESC desc { };
        desc.Width = m_pSurface->w;
        desc.Height = m_pSurface->h;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        // Create subresource data
        D3D11_SUBRESOURCE_DATA initData { };
        initData.pSysMem = m_pSurfacePixels;
        initData.SysMemPitch = m_pSurface->pitch;
        initData.SysMemSlicePitch = m_pSurface->pitch * m_pSurface->h;

        // Create texture
        ID3D11Texture2D* pTexture { nullptr };
        HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &pTexture);

        if (FAILED(hr))
        {
            std::cerr << "Failed to create D3D11 texture\n";
            return nullptr;
        }

        // Create shader resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc { };
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;

        hr = pDevice->CreateShaderResourceView(pTexture, &srvDesc, &m_pShaderResourceView);

        // Release the texture (the SRV holds a reference)
        pTexture->Release();

        if (FAILED(hr))
        {
            std::cerr << "Failed to create shader resource view\n";
            return nullptr;
        }

        return m_pShaderResourceView;
    }

    ID3D11ShaderResourceView* Texture::GetShaderResourceView() const
    {
        return m_pShaderResourceView;
    }

    ColorRGB Texture::Sample(float u, float v) const
    {
        // Clamp to ensure [0, 1] range
        u = std::clamp(u, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);

        const int width { m_pSurface->w };
        const int height { m_pSurface->h };

        const int x { static_cast<int>(u * (width - 1)) };
        const int y { static_cast<int>(v * (height - 1)) };

        const uint32_t pixel { m_pSurfacePixels[x + (y * width)] };

        return ToColorRGB(pixel, m_pSurface->format);
    }
}