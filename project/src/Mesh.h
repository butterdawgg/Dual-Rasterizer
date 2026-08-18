#pragma once

#include <string>
#include <vector>

#include "DXIncludes.h"

#include "Maths.h"
#include "Bounds.h"

namespace dae
{
    struct Vertex_Out;

    struct Vertex
    {
        Vector3 position { };
        ColorRGB color { colors::White };
        Vector2 uv { };
        Vector3 normal { };
        Vector3 tangent { };
        Vector3 viewDirection { };

        /* Converts the vertex to a Vertex_Out using 'w = 1.0f' */
        Vertex_Out ToVertexOut() const;
    };

    struct Vertex_Out
    {
        Vector4 position { };
        ColorRGB color { colors::White };
        Vector2 uv { };
        Vector3 normal { };
        Vector3 tangent { };
        Vector3 viewDirection { };
    };

    enum class Topology
    {
        TriangleList,
        TriangleStrip
    };

    /* Represents DX specific data of a mesh */
    struct MeshDX
    {
        ID3D11Buffer* pVertexBuffer { nullptr };
        ID3D11Buffer* pIndexBuffer { nullptr };
        uint32_t numIndices { 0 };
    };

    class Mesh
    {
        private:

        std::vector<Vertex> m_Vertices { };
        std::vector<uint32_t> m_Indices { };

        Topology m_PrimitiveTopology { Topology::TriangleList };

        unsigned int m_MaterialId { };

        Vector3 m_Position { 0.0f, 0.0f, 0.0f };
        Vector3 m_Rotation { 0.0f, 0.0f, 0.0f };
        Vector3 m_Scale { 1.0f, 1.0f, 1.0f };

        Bounds m_LocalBounds { };

        bool m_ForceRenderingOff { };

        // Cache the world matrix
        mutable Matrix m_WorldMatrix { };
        mutable bool m_WorldMatrixDirty { };

        // Cache the world AABB
        mutable Bounds m_WorldBounds { };

        /* Recalculates the world matrix using position, rotation and scale parameters */
        void UpdateWorldMatrixCache() const;

        /* Recalculates the world bounds using the world matrix */
        void UpdateWorldBoundsCache() const;

        // --- DX11 specific ---
        MeshDX m_MeshDX { };
        bool m_IsTransparent { };

        public:

        explicit Mesh(const std::vector<Vertex>& vertices,
            const std::vector<uint32_t>& indices,
            unsigned int materialId = 0,
            Topology primitiveTopology = Topology::TriangleList);

        explicit Mesh(const std::string& filepath, unsigned int materialId = 0,
            Topology primitiveTopology = Topology::TriangleList);

        /* Sets the position of the mesh and marks worldMatrix as dirty */
        void SetPosition(const Vector3& position);
        /* Sets the rotation of the mesh and marks worldMatrix as dirty */
        void SetRotation(const Vector3& rotation);
        /* Sets the scale of the mesh and marks worldMatrix as dirty */
        void SetScale(const Vector3& scale);
        /* Sets the material index used for shading */
        void SetMaterialID(unsigned int id);
        /* Sets if the mesh is transparent (DirectX only) */
        void SetTransparent(bool transparent);
        /* Sets if rendering this mesh should be disabled */
        void SetForceRenderingOff(bool forceRenderingOff);

        /* Returns the position of the mesh */
        Vector3 GetPosition() const;
        /* Returns the rotation of the mesh */
        Vector3 GetRotation() const;
        /* Returns the scale of the mesh */
        Vector3 GetScale() const;
        /* Returns the material index used for shading */
        unsigned int GetMaterialID() const;

        /* Updates the world matrix cache if marked dirty and returns the result */
        const Matrix& GetWorldMatrix() const;
        /* Updates the world bounds cache if marked dirty and returns the result */
        const Bounds& GetWorldBounds() const;

        /* Returns the vertices in the local space of the mesh */
        const std::vector<Vertex>& GetVertices() const;
        /* Returns the indices of the mesh */
        const std::vector<uint32_t>& GetIndices() const;
        /* Returns the primitive topology used by the mesh */
        Topology GetPrimitiveTopology() const;

        /* Returns the DX specific data of the mesh */
        MeshDX& GetMeshDX();
        /* Returns if the mesh is transparent (DirectX only) */
        bool IsTransparent() const;
        /* Returns if rendering this mesh is enabled */
        bool GetForceRenderingOff() const;
    };
}