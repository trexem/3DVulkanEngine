#include "Octree.hpp"

namespace engine {
    Octree::Octree(const glm::vec3& position, float size, int maxDepth) : 
        m_position(position), 
        m_size(size), 
        m_voxel(nullptr), 
        m_children(nullptr) {
            root = std::make_shared<Voxel>(VoxelType::Air);
        }

    void Octree::subdivide(std::shared_ptr<Voxel>& voxel, float currentSize, int currentDepth) {
        if (voxel->isSubdivided || currentDepth >= maxDepth) return;

        voxel->isSubdivided = true;
        float childSize = currentSize / 2.0f;

        for (int i = 0; i < 8; ++i) {
            voxel->children.push_back(std::make_shared<Voxel>(voxel->type));
        }

        for (auto& child : voxel->children) {
            if (shouldSubdivide(child, currentSize, currentDepth)) {
                subdivide(child, childSize, currentDepth + 1);
            }
            
        }
    }

    bool Octree::shouldSubdivide(const std::shared_ptr<Voxel>& voxel, float currentSize, int currentDepth) {

    }

    void Octree::setVoxelType(std::shared_ptr<Voxel>& voxel, VoxelType type) {
        voxel->type = type;
    }

    void Octree::setVoxelTypeAt(const glm::vec3& position, VoxelType type) {
        auto voxel = getVoxelAt(position, m_size, root, m_size);
        setVoxelType(voxel, type);
    }

    std::shared_ptr<Voxel> Octree::getVoxelAt(
        const glm::vec3& position, 
        float voxelSize,
        std::shared_ptr<Voxel> currentNode, 
        float currentNodeSize
    ) {
        if (!currentNode->isSubdivided) return currentNode;

        float halfSize = currentNodeSize / 2.0f;
        int index = (position.x >= halfSize) << 2 | (position.y >= halfSize) << 1 | (position.z >= halfSize);
        glm::vec3 newPos = position - glm::vec3((position.x >= halfSize) * halfSize, (position.y >= halfSize) * halfSize, (position.z >= halfSize) * halfSize);

        return getVoxelAt(newPos, voxelSize, currentNode->children[index], halfSize);
    }
} //namespace engine
