#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace engine {
    enum class VoxelType {
        Air,
        Stone,
        Dirt,
        Wood,
        Leaf,

    };

    struct Voxel
    {
        VoxelType type;
        bool isSubdivided { false };
        std::vector<std::shared_ptr<Voxel>> children;

        Voxel(VoxelType type = VoxelType::Air) : type(type), isSubdivided(false) {}
    };
} //namepsace engine
