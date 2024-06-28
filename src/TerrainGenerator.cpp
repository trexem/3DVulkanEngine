#include "TerrainGenerator.hpp"

namespace engine {
    void TerrainGenerator::generateTerrain(Octree& octree, const glm::vec3& position, float size) {
        for (float x = 0; x < 100; x++) {
            for (float y = 0; y < 100; y++) {
                for (float z = 0; z < 100; z++) {
                    Voxel voxel = { 0, true };
                    octree.insertVoxel(glm::vec3(x, y, z), voxel);
                }
            }
        }
    }
} //namespace engine
