#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "DXIncludes.h"

#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "Light.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
    class Timer;

    enum class RenderMode
    {
        ObservedArea = 0,
        DiffuseOnly = 1,
        SpecularOnly = 2,
        Combined = 3
    };

    enum class SamplingMode
    {
        Point = 0,
        Linear = 1,
        Anisotropic = 2
    };

    class Renderer final
    {
        public:

        Renderer(SDL_Window* pWindow);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer(Renderer&&) noexcept = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer& operator=(Renderer&&) noexcept = delete;

        void Update(Timer* pTimer);

        void Render();

        void ToggleRenderingBackend(SDL_Scancode key);
        void ToggleMeshRotation(SDL_Scancode key);
        void ToggleUniformClearColor(SDL_Scancode key);

        void ToggleMeshRendering(SDL_Scancode key);
        void ToggleSamplingMode(SDL_Scancode key);

        void ToggleRenderMode(SDL_Scancode key);
        void ToggleNormalMapping(SDL_Scancode key);
        void ToggleDrawDepth(SDL_Scancode key);

        bool SaveBufferToImage() const;

        private:

        SDL_Window* m_pWindow { };

        SDL_Surface* m_pFrontBuffer { nullptr };
        SDL_Surface* m_pBackBuffer { nullptr };
        uint32_t* m_pBackBufferPixels { };

        float* m_pDepthBufferPixels { };

        int m_Width { };
        int m_Height { };

        std::unique_ptr<Camera> m_Camera;
        std::vector<Mesh> m_Meshes { };
        std::vector<Material> m_Materials { };
        std::vector<Light> m_Lights { };

        // Rendering parameters
        RenderMode m_RenderMode { RenderMode::Combined };
        SamplingMode m_SamplingMode { SamplingMode::Point };
        bool m_DrawDepth { false };
        bool m_NormalMapping { true };
        bool m_RotateMesh { false };
        bool m_DXRenderingEnabled { false };

        bool m_UniformClearColor { false };
        ColorRGB m_ClearColor { colors::LightGray };

        void RenderSoftware();
        void DrawDepthBuffer();

        // --- DX11 specific ---

        // Initialization flag
        bool m_InitializedDX { };

        // Device and context
        ID3D11Device* m_pDevice { nullptr };
        ID3D11DeviceContext* m_pDeviceContext { nullptr };

        // Swap chain
        IDXGISwapChain* m_pSwapChain { nullptr };

        // Render target
        ID3D11Texture2D* m_pRenderTargetBuffer { nullptr };
        ID3D11RenderTargetView* m_pRenderTargetView { nullptr };

        // Depth stencil
        ID3D11Texture2D* m_pDepthStencilBuffer { nullptr };
        ID3D11DepthStencilView* m_pDepthStencilView { nullptr };

        // Effect
        ID3DX11Effect* m_pEffect { nullptr };
        ID3DX11EffectTechnique* m_pOpaqueTechnique { nullptr };
        ID3DX11EffectTechnique* m_pTransparentTechnique { nullptr };
        ID3D11InputLayout* m_pInputLayout { nullptr };

        // Effect flags
        ID3DX11EffectScalarVariable* m_pUseNormalMap { nullptr };
        ID3DX11EffectScalarVariable* m_pSamplingMode { nullptr };

        HRESULT InitializeDX();
        void InitializeMeshesDX();
        void InitializeMaterialsDX();
        void RenderDX();
        void CleanupDX();
    };
}
