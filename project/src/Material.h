#pragma once

#include "Maths.h"
#include "Texture.h"

#include <memory>

#include "DXIncludes.h"


namespace dae
{
    struct MaterialDX
    {
        // Matrices
        ID3DX11EffectMatrixVariable* pMatWorld { nullptr };
        ID3DX11EffectMatrixVariable* pMatViewProj { nullptr };

        // Textures
        ID3DX11EffectShaderResourceVariable* pDiffuseMap { nullptr };
        ID3DX11EffectShaderResourceVariable* pNormalMap { nullptr };
        ID3DX11EffectShaderResourceVariable* pSpecularMap { nullptr };
        ID3DX11EffectShaderResourceVariable* pGlossinessMap { nullptr };

        // Camera
        ID3DX11EffectVectorVariable* pCameraPosition { nullptr };

        // Light array
        ID3DX11EffectScalarVariable* pNumLights { nullptr };
        ID3DX11EffectVariable* pLightsArray { nullptr };

        // Material properties
        ID3DX11EffectVectorVariable* pDiffuseColor { nullptr };
        ID3DX11EffectVectorVariable* pSpecularColor { nullptr };
        ID3DX11EffectVectorVariable* pAmbientColor { nullptr };
        ID3DX11EffectScalarVariable* pDiffuseReflectance { nullptr };
        ID3DX11EffectScalarVariable* pSpecularReflectance { nullptr };
        ID3DX11EffectScalarVariable* pGlossiness { nullptr };
    };

    // Blinn-Phong shading material
    struct Material
    {
        ColorRGB diffuseColor { colors::White };
        ColorRGB specularColor { colors::White };
        ColorRGB ambientColor { colors::White * 0.1f };
        float diffuseReflectance { 1.0f };
        float specularReflectance { 0.5f };
        float glossiness { 25.0f };

        std::unique_ptr<Texture> diffuseColorMap { };
        std::unique_ptr<Texture> specularMap { };
        std::unique_ptr<Texture> glossinessMap { };
        std::unique_ptr<Texture> normalMap { };

        explicit Material() = default;

        explicit Material(
            const ColorRGB& diffuseColor,
            const ColorRGB& specularColor,
            const ColorRGB& ambientColor,
            float diffuseReflectance,
            float specular,
            float glossiness
        );

        explicit Material(
            const ColorRGB& diffuseColor,
            const ColorRGB& specularColor,
            const ColorRGB& ambientColor,
            float diffuseReflectance,
            float specularReflectance,
            float glossiness,
            Texture* diffuseColorMap,
            Texture* specularMap,
            Texture* glossinessMap,
            Texture* normalMap
        );

        ColorRGB GetAmbient(const Vector2& uv) const;

        ColorRGB GetObservedArea(const Vector3& n, const Vector3& l) const;

        ColorRGB GetDiffuse(const Vector3& n, const Vector3& l, const Vector2& uv) const;

        ColorRGB GetSpecular(const Vector3& n, const Vector3& l,
            const Vector3& v, const Vector2& uv) const;

        Vector3 GetNormal(const Vector3& n, const Vector3& t, const Vector2& uv) const;

        MaterialDX& GetMaterialDX();

        private:

        MaterialDX m_MaterialDX { };
    };
}