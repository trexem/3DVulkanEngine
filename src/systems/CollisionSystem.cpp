#include "CollisionSystem.hpp"

#include "Components.hpp"

namespace engine {

    CollisionSystem::CollisionSystem() {

    }

    CollisionSystem::~CollisionSystem() {

    }

    void CollisionSystem::update(FrameInfo& frameInfo) {
        EntityManager& eManager = frameInfo.entityManager;
        auto physicsEntities = eManager.getEntitiesWithComponent(ComponentType::Physics);
        for (int i = 0; i < physicsEntities.size(); i++) {
            auto entityA = physicsEntities.at(i);
            if (eManager.hasComponent<ModelComponent>(entityA)) {
                const auto& modelCompA = eManager.getComponentData<ModelComponent>(entityA);
                const auto& physicsCompA = eManager.getComponentData<PhysicsComponent>(entityA);
                const auto& transformCompA = eManager.getComponentData<TransformComponent>(entityA);
                auto boundingBoxA = modelCompA.model->getBoundingBox();
                boundingBoxA.min = boundingBoxA.min * transformCompA.scale;
                boundingBoxA.max = boundingBoxA.max * transformCompA.scale;
                for (int j = i; j < physicsEntities.size(); j++) {
                    auto entityB = physicsEntities.at(j);
                    if (eManager.hasComponent<ModelComponent>(entityB) 
                        && entityA != entityB) {
                        const auto& modelCompB = eManager.getComponentData<ModelComponent>(entityB);
                        const auto& physicsCompB = eManager.getComponentData<PhysicsComponent>(entityB);
                        const auto& transformCompB = eManager.getComponentData<TransformComponent>(entityB);
                        auto boundingBoxB = modelCompB.model->getBoundingBox();
                        boundingBoxB.min = boundingBoxB.min * transformCompB.scale;
                        boundingBoxB.max = boundingBoxB.max * transformCompB.scale;

                        // Check for collision between entityA and entityB
                        if (checkCollision(transformCompA, boundingBoxA, transformCompB, boundingBoxB)) {
                            // Handle collision between entityA and entityB
                            handleCollision(entityA, entityB, eManager);
                        }
                    }
                }
            }
        }
    }

    bool CollisionSystem::checkCollision(const TransformComponent& transformCompA, const BoundingBox& boundingBoxA,
                                const TransformComponent& transformCompB, const BoundingBox& boundingBoxB) {
        
        return (transformCompA.translation.x + boundingBoxA.max.x >= transformCompB.translation.x + boundingBoxB.min.x &&
                transformCompA.translation.x + boundingBoxA.min.x <= transformCompB.translation.x + boundingBoxB.max.x &&
                transformCompA.translation.y + boundingBoxA.max.y >= transformCompB.translation.y + boundingBoxB.min.y &&
                transformCompA.translation.y + boundingBoxA.min.y <= transformCompB.translation.y + boundingBoxB.max.y &&
                transformCompA.translation.z + boundingBoxA.max.z >= transformCompB.translation.z + boundingBoxB.min.z &&
                transformCompA.translation.z + boundingBoxA.min.z <= transformCompB.translation.z + boundingBoxB.max.z);
    }

    void CollisionSystem::handleCollision(uint32_t entityA, uint32_t entityB, EntityManager& eManager) {
        PhysicsComponent physicsCompA = eManager.getComponentData<PhysicsComponent>(entityA);
        PhysicsComponent physicsCompB = eManager.getComponentData<PhysicsComponent>(entityB);
        TransformComponent tCompA = eManager.getComponentData<TransformComponent>(entityA);
        TransformComponent tCompB = eManager.getComponentData<TransformComponent>(entityB);

        if (physicsCompA.velocity.y > -.2 && physicsCompA.velocity.y < 0.2
            && !physicsCompB.movable) {
            physicsCompA.grounded = true;
            physicsCompA.velocity.y = 0;
        } else {
            physicsCompA.grounded = false;
        }

        if (physicsCompB.velocity.y > -.2 && physicsCompB.velocity.y <= 0 && !physicsCompA.movable) {
            physicsCompB.grounded = true;
            physicsCompB.velocity.y = 0;
        } else {
            physicsCompB.grounded = false;
        }
        
        float massA = physicsCompA.mass;
        if (!physicsCompA.movable) {
            if (physicsCompB.grounded) {
                eManager.setComponentData(entityB,physicsCompB);
                return;
            }
            massA = 999999.f;
        }

        float massB = physicsCompA.mass;
        if (!physicsCompB.movable) {
            if (physicsCompA.grounded) {
                eManager.setComponentData(entityA,physicsCompA);
                return;
            }
            massB = 999999.f;
        }

        float mTotal = massA + massB;
        glm::vec3 vRel = physicsCompB.velocity - physicsCompA.velocity;

        float avgCoefRes = 0.5f * (physicsCompA.coefRes + physicsCompB.coefRes);

        glm::vec3 impulse = (1.f + avgCoefRes) * ( massA * massB / mTotal) * vRel;
        glm::vec3 mtv = calculateMTV(entityA, entityB, eManager);

        if (physicsCompA.movable) {
            physicsCompA.velocity += (impulse / massA);
            if (physicsCompB.movable) {
                physicsCompB.velocity -= (impulse / massA);
                tCompA.translation += 0.5f * mtv;
                tCompB.translation -= 0.5f * mtv;
            } else {
                physicsCompB.velocity = glm::vec3{0,0,0};
                tCompA.translation += mtv;
            }
        } else {
            physicsCompA.velocity = glm::vec3{0,0,0};
            if (physicsCompB.movable) {
                physicsCompB.velocity -= (impulse / massA);
                tCompB.translation -= mtv;
            } else {
                physicsCompB.velocity = glm::vec3{0,0,0};
            }
        }
        eManager.setComponentData(entityA,physicsCompA);
        eManager.setComponentData(entityA, tCompA);
        eManager.setComponentData(entityB,physicsCompB);
        eManager.setComponentData(entityB, tCompB);
    }

    glm::vec3 CollisionSystem::calculateMTV(uint32_t entityA, uint32_t entityB, EntityManager& eManager) {
        std::shared_ptr<Model> modelA = eManager.getComponentData<ModelComponent>(entityA).model;
        std::shared_ptr<Model> modelB = eManager.getComponentData<ModelComponent>(entityB).model;

        // Get the bounding boxes
        BoundingBox bboxA = modelA->getBoundingBox();
        BoundingBox bboxB = modelB->getBoundingBox();

        // Get the world positions of the objects
        TransformComponent tCompA = eManager.getComponentData<TransformComponent>(entityA);
        TransformComponent tCompB = eManager.getComponentData<TransformComponent>(entityB);
        glm::vec3 positionA = tCompA.translation;
        glm::vec3 positionB = tCompB.translation;
        bboxA.scale(tCompA.scale);
        bboxB.scale(tCompB.scale);

        // Transform the bounding boxes to world space
        bboxA.min += positionA;
        bboxA.max += positionA;
        bboxB.min += positionB;
        bboxB.max += positionB;

        // Calculate the overlap along each axis
        float xOverlap = std::min(bboxA.max.x - bboxB.min.x, bboxB.max.x - bboxA.min.x);
        float yOverlap = -std::min(bboxA.max.y - bboxB.min.y, bboxB.max.y - bboxA.min.y);
        float zOverlap = std::min(bboxA.max.z - bboxB.min.z, bboxB.max.z - bboxA.min.z);

        // Find the axis with the smallest overlap
        if (std::abs(xOverlap) < std::abs(yOverlap) && std::abs(xOverlap) < std::abs(zOverlap)) {
            return glm::vec3(xOverlap, 0.0f, 0.0f);  // MTV along the x-axis
        } else if (std::abs(yOverlap) < std::abs(zOverlap)) {
            return glm::vec3(0.0f, yOverlap, 0.0f);  // MTV along the y-axis
        } else {
            return glm::vec3(0.0f, 0.0f, zOverlap);  // MTV along the z-axis
        }
    }
} // namespace engine
