#include "VoxelSystem.hpp"

namespace engine {
    VoxelSystem::VoxelSystem() {}
    VoxelSystem::~VoxelSystem() {}

    void VoxelSystem::update(FrameInfo& frameInfo) {
        for (const uint32_t entityID : frameInfo.entityManager.getEntitiesWithComponent(ComponentType::Octree)) {
            if (frameInfo.entityManager.entityExists(entityID)) {
                OctreeComponent& octreeComp = 
                    frameInfo.entityManager.getComponentData<OctreeComponent>(entityID);
            }
        }
    }
} //namespace engine
