#include "PhysicsSystem.hpp"

namespace engine {

    PhysicsSystem::PhysicsSystem() {

    }

    PhysicsSystem::~PhysicsSystem() {

    }

    void PhysicsSystem::update(FrameInfo& frameInfo) {
        for (const uint32_t entityID : frameInfo.entityManager.getEntitiesWithComponent(ComponentType::Physics)) {
            if (frameInfo.entityManager.entityExists(entityID)) {
                PhysicsComponent physComp = 
                    frameInfo.entityManager.getComponentData<PhysicsComponent>(entityID);
                TransformComponent transComp = 
                    frameInfo.entityManager.getComponentData<TransformComponent>(entityID);
                PhysicsComponent updatedphysComp = physComp;
                updatedphysComp.velocity += physComp.acceleration * frameInfo.frameTime;
                transComp.translation += physComp.velocity * frameInfo.frameTime;

                frameInfo.entityManager.setComponentData(entityID, updatedphysComp);
                frameInfo.entityManager.setComponentData(entityID, transComp);
            }
        }
    }
}