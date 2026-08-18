#include "Mesh.h"
#include "Bounds.h"
#include "Utils.h"

namespace dae
{
    Vertex_Out Vertex::ToVertexOut() const
    {
        Vertex_Out result { };

        result.position = position.ToPoint4();
        result.color = color;
        result.uv = uv;
        result.normal = normal;
        result.tangent = tangent;
        result.viewDirection = viewDirection;

        return result;
    }

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
        unsigned int materialId, Topology primitiveTopology) : m_Vertices(vertices), m_Indices(indices),
        m_MaterialId(materialId), m_PrimitiveTopology(primitiveTopology)
    {
        UpdateWorldMatrixCache();

        m_LocalBounds.Fit(m_Vertices);
    }

    Mesh::Mesh(const std::string& filepath, unsigned int materialId, Topology primitiveTopology) :
        m_MaterialId(materialId), m_PrimitiveTopology(primitiveTopology)
    {
        Utils::ParseOBJ(filepath, m_Vertices, m_Indices);

        UpdateWorldMatrixCache();

        m_LocalBounds.Fit(m_Vertices);
    }

    void Mesh::UpdateWorldMatrixCache() const
    {
        m_WorldMatrix =
            Matrix::CreateScale(m_Scale) *
            Matrix::CreateRotation(m_Rotation) *
            Matrix::CreateTranslation(m_Position);

        m_WorldMatrixDirty = false;
    }

    void Mesh::UpdateWorldBoundsCache() const
    {
        if (m_WorldMatrixDirty)
            UpdateWorldMatrixCache();

        const Vector3 localCenter { (m_LocalBounds.min + m_LocalBounds.max) * 0.5f };
        const Vector3 localExtent { (m_LocalBounds.max - m_LocalBounds.min) * 0.5f };

        const Matrix absRot {
            Vector3 { m_WorldMatrix.GetAxisX().Abs() },
            Vector3 { m_WorldMatrix.GetAxisY().Abs() },
            Vector3 { m_WorldMatrix.GetAxisZ().Abs() },
            Vector3 { }
        };

        const Vector3 worldCenter { m_WorldMatrix.TransformPoint(localCenter) };
        const Vector3 worldExtent { absRot.TransformVector(localExtent) };

        m_WorldBounds.min = worldCenter - worldExtent;
        m_WorldBounds.max = worldCenter + worldExtent;
    }

    void Mesh::SetPosition(const Vector3& position)
    {
        if (m_Position == position)
            return;

        m_Position = position;

        m_WorldMatrixDirty = true;
    }

    void Mesh::SetRotation(const Vector3& rotation)
    {
        if (m_Rotation == rotation)
            return;

        m_Rotation = rotation;

        m_WorldMatrixDirty = true;
    }

    void Mesh::SetScale(const Vector3& scale)
    {
        if (m_Scale == scale)
            return;

        m_Scale = scale;

        m_WorldMatrixDirty = true;
    }

    void Mesh::SetMaterialID(unsigned int id)
    {
        m_MaterialId = id;
    }

    void Mesh::SetTransparent(bool transparent)
    {
        m_IsTransparent = transparent;
    }

    void Mesh::SetForceRenderingOff(bool forceRenderingOff)
    {
        m_ForceRenderingOff = forceRenderingOff;
    }

    Vector3 Mesh::GetPosition() const
    {
        return m_Position;
    }

    Vector3 Mesh::GetRotation() const
    {
        return m_Rotation;
    }

    Vector3 Mesh::GetScale() const
    {
        return m_Scale;
    }

    unsigned int Mesh::GetMaterialID() const
    {
        return m_MaterialId;
    }

    const Matrix& Mesh::GetWorldMatrix() const
    {
        if (m_WorldMatrixDirty)
            UpdateWorldMatrixCache();

        return m_WorldMatrix;
    }

    const Bounds& Mesh::GetWorldBounds() const
    {
        if (m_WorldMatrixDirty)
            UpdateWorldBoundsCache();

        return m_WorldBounds;
    }

    const std::vector<Vertex>& Mesh::GetVertices() const
    {
        return m_Vertices;
    }

    const std::vector<uint32_t>& Mesh::GetIndices() const
    {
        return m_Indices;
    }

    Topology Mesh::GetPrimitiveTopology() const
    {
        return m_PrimitiveTopology;
    }

    MeshDX& Mesh::GetMeshDX()
    {
        return m_MeshDX;
    }

    bool Mesh::IsTransparent() const
    {
        return m_IsTransparent;
    }

    bool Mesh::GetForceRenderingOff() const
    {
        return m_ForceRenderingOff;
    }
}