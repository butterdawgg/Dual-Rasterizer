#include "Bounds.h"

#include "Mesh.h"

namespace dae
{
    Bounds::Bounds(const Vector3& min, const Vector3& max) : min(min), max(max)
    { }

    void Bounds::Fit(std::vector<Vertex>& vertices)
    {
        if (vertices.size() < 1)
            return;

        min = vertices[0].position;
        max = vertices[0].position;

        for (auto& vertex : vertices)
        {
            const Vector3 pos { vertex.position };

            if (pos.x < min.x) min.x = pos.x;
            if (pos.y < min.y) min.y = pos.y;
            if (pos.z < min.z) min.z = pos.z;

            if (pos.x > max.x) max.x = pos.x;
            if (pos.y > max.y) max.y = pos.y;
            if (pos.z > max.z) max.z = pos.z;
        }
    }
}