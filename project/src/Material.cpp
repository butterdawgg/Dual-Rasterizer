#include "Material.h"
#include "Light.h"

namespace dae
{
    Material::Material(
        const ColorRGB& diffuseColor,
        const ColorRGB& specularColor,
        const ColorRGB& ambientColor,
        float diffuseReflectance,
        float specular,
        float glossiness)
        : diffuseColor(diffuseColor)
        , specularColor(specularColor)
        , ambientColor(ambientColor)
        , diffuseReflectance(diffuseReflectance)
        , specularReflectance(specular)
        , glossiness(glossiness)
        , diffuseColorMap(nullptr)
        , specularMap(nullptr)
        , glossinessMap(nullptr)
        , normalMap(nullptr)
    { }

    Material::Material(
        const ColorRGB& diffuseColor,
        const ColorRGB& specularColor,
        const ColorRGB& ambientColor,
        float diffuseReflectance,
        float specularReflectance,
        float glossiness,
        Texture* diffuseColorMap,
        Texture* specularMap,
        Texture* glossinessMap,
        Texture* normalMap)
        : diffuseColor(diffuseColor)
        , specularColor(specularColor)
        , ambientColor(ambientColor)
        , diffuseReflectance(diffuseReflectance)
        , specularReflectance(specularReflectance)
        , glossiness(glossiness)
        , diffuseColorMap(diffuseColorMap)
        , specularMap(specularMap)
        , glossinessMap(glossinessMap)
        , normalMap(normalMap)
    { }

    ColorRGB Material::GetAmbient(const Vector2& uv) const
    {
        ColorRGB ambient { ambientColor };

        if (diffuseColorMap != nullptr)
            ambient *= diffuseColorMap->Sample(uv);

        return ambient;
    }

    ColorRGB Material::GetObservedArea(const Vector3& n, const Vector3& l) const
    {
        const float nDotL { std::max(Vector3::Dot(n, l), 0.0f) };

        return ColorRGB { nDotL, nDotL, nDotL };
    }

    ColorRGB Material::GetDiffuse(const Vector3& n, const Vector3& l, const Vector2& uv) const
    {
        const float nDotL { std::max(Vector3::Dot(n, l), 0.0f) };

        ColorRGB kd { diffuseColor };
        if (diffuseColorMap != nullptr)
            kd *= diffuseColorMap->Sample(uv);

        return kd * diffuseReflectance * nDotL * INV_PI;
    }

    ColorRGB Material::GetSpecular(const Vector3& n, const Vector3& l,
        const Vector3& v, const Vector2& uv) const
    {
        const Vector3 h { (l + v).SafeNormalized() };

        const float nDotL = std::max(Vector3::Dot(n, l), 0.0f);
        const float nDotH = std::max(Vector3::Dot(n, h), 0.0f);

        ColorRGB ks { specularColor };
        if (specularMap != nullptr)
            ks *= specularMap->Sample(uv);

        float phongExp = glossiness;
        if (glossinessMap != nullptr)
            phongExp *= glossinessMap->Sample(uv).r;

        float spec = 0.0f;
        if (nDotL > 0.0f)
            spec = nDotL * std::pow(nDotH, phongExp) * specularReflectance;

        return ks * spec;
    }

    Vector3 Material::GetNormal(const Vector3& n, const Vector3& t, const Vector2& uv) const
    {
        if (normalMap == nullptr)
            return n;

        // Compute binormal
        const Vector3 b { Vector3::Cross(n, t).SafeNormalized() };

        // Construct the TBN matrix
        Matrix TBN {
            Vector4 { t.x, t.y, t.z, 0.0f },
            Vector4 { b.x, b.y, b.z, 0.0f },
            Vector4 { n.x, n.y, n.z, 0.0f },
            Vector4 { 0.f, 0.f, 0.f, 1.0f }
        };

        // Sample the normal map, and remap to [-1,1]
        const Vector3 normalMapSample { ToVector3(normalMap->Sample(uv)) * 2.0f -
            Vector3 { 1.0f, 1.0f, 1.0f } };

        // Compute and return the world space normal
        return TBN.TransformVector(normalMapSample).SafeNormalized();
    }

    MaterialDX& Material::GetMaterialDX()
    {
        return m_MaterialDX;
    }
}