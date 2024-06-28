#pragma once

#include "FrameInfo.hpp"

namespace engine {

    class VoxelSystem {
    public:
        VoxelSystem();
        ~VoxelSystem();

        VoxelSystem(const VoxelSystem&) = delete;
		VoxelSystem& operator=(const VoxelSystem&) = delete;

        void update(FrameInfo& frameInfo);
    };
} //namespace engine
