#pragma once

#include "Voxel.hpp"

namespace engine {

    class Octree {
    public:
        std::shared_ptr<Voxel> root;
        float size;
        int maxDepth;
        Octree(const glm::vec3& position, float size, int maxDepth);

        Octree(const Octree&) = delete;
        Octree& operator=(const Octree&) = delete;
        
        Octree(Octree&& other) noexcept : 
            m_position(other.m_position), 
            m_size(other.m_size), 
            m_voxel(std::move(other.m_voxel)),
            m_children(std::move(other.m_children)) {}

        Octree& operator=(Octree&& other) noexcept {
            if (this != &other) {
                m_position = other.m_position;
                m_size = other.m_size;
                m_voxel = std::move(other.m_voxel);
                m_children = std::move(other.m_children);
            }
            return *this;
        }

        void setVoxelTypeAt(const glm::vec3& position, VoxelType type);
        std::shared_ptr<Voxel> getVoxelAt(
            const glm::vec3& position, float voxelSize,
            std::shared_ptr<Voxel> currentNode, float currentNodeSize);

        void subdivide(std::shared_ptr<Voxel>& voxel, float currentSize, int currentDepth);
        bool shouldSubdivide(const std::shared_ptr<Voxel>& voxel, float currentSize, int currentDepth);
        void setVoxelType(std::shared_ptr<Voxel>& voxel, VoxelType type);

    private:
        glm::vec3 m_position;
        float m_size;
        std::unique_ptr<Voxel> m_voxel;
        std::unique_ptr<Octree[]> m_children;
    };
} //namepsace engine
