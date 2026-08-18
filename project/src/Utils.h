#pragma once

#include <cassert>
#include <fstream>
#include <vector>

#include "Maths.h"

namespace dae
{
    struct Vertex;

    namespace Utils
    {
        // Just parses vertices and indices
        bool ParseOBJ(const std::string& filename, std::vector<Vertex>& vertices,
            std::vector<uint32_t>& indices, bool flipAxisAndWinding = true);
    }
}