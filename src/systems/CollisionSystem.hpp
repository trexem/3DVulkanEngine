#pragma once

#include "FrameInfo.hpp"

namespace engine {

    class CollisionSystem {
    public:
        CollisionSystem();
        ~CollisionSystem();

        CollisionSystem(const CollisionSystem&) = delete;
        CollisionSystem& operator=(const CollisionSystem&) = delete;

        void update(FrameInfo& frameInfo);
        static glm::vec3 calculateMTV(uint32_t entityA, uint32_t entityB, EntityManager& eManager);

    private:
        static bool checkCollision(const TransformComponent& transformCompA, const BoundingBox& bBoxA,
                                    const TransformComponent& transformCompB, const BoundingBox& bBoxB);

        static void handleCollision(uint32_t entityA, uint32_t entityB, EntityManager& eManager);
    };
} // namespace engine
