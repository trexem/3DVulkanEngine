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
                if (physComp.hasGravity && !physComp.grounded) {
                    std::cout << "Moving with gravity: " << entityID << std::endl;
                    physComp.velocity += (physComp.acceleration + physComp.gravity) * frameInfo.frameTime;
                    std::cout << "vel(" << physComp.velocity.x << ", " << physComp.velocity.y << ", " << physComp.velocity.z << ")" << std::endl;
                } else {
                    std::cout << "Moving without gravity: " << entityID << std::endl;
                    physComp.velocity += physComp.acceleration * frameInfo.frameTime;
                    std::cout << "vel(" << physComp.velocity.x << ", " << physComp.velocity.y << ", " << physComp.velocity.z << ")" << std::endl;
                }
                transComp.translation += physComp.velocity * frameInfo.frameTime;

                physComp.grounded = false;

                frameInfo.entityManager.setComponentData(entityID, physComp);
                frameInfo.entityManager.setComponentData(entityID, transComp);
            }
        }
    }
}