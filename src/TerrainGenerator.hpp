#pragma once

#include "Octree.hpp"

namespace engine {
    class TerrainGenerator
    {
    public:
        TerrainGenerator();
        ~TerrainGenerator();

        static void generateTerrain(Octree& octree, const glm::vec3& position, float size);
    };
} //namespace engine
