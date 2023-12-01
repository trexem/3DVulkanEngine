#pragma once

#include "FrameInfo.hpp"

namespace engine {
    class PhysicsSystem {
    public:
        PhysicsSystem();
        ~PhysicsSystem();

		PhysicsSystem(const PhysicsSystem&) = delete;
		PhysicsSystem& operator=(const PhysicsSystem&) = delete;
        void update(FrameInfo& frameInfo);
    };
} //namespace engine
