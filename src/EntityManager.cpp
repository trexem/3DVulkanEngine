#include "EntityManager.hpp"

#include <cassert>
#include <iostream>


namespace engine {
    EntityManager::EntityManager(size_t t_maxEntities, Device& device) : maxEntities{t_maxEntities} {
        entities.reserve(maxEntities);
        entityComponentMasks.resize(maxEntities);
        componentPools.resize(static_cast<size_t>(ComponentType::Count));
        noTexture = std::make_shared<Image>(device, "textures/noTexture.png", 0);
        noTextureComp.imagesIndex.push_back(0);
        noTextureComp.textureInfo.push_back(noTexture->textureInfo());
    }

    EntityManager::~EntityManager() {
        // Release the memory associated with component pools
        for (auto& componentPool : componentPools) {
            componentPool.clear(); // This will release the unique_ptr and its associated memory
        }
    }
    
    uint32_t EntityManager::createEntity() {
        if (entityCount < maxEntities) {
            uint32_t newEntityID = findAvailableEntityID();
            entities.push_back(newEntityID);
            entityCount++;
            return newEntityID;
        } else {
            return 0; // or error code
        }
    }
    
    void EntityManager::destroyEntity(uint32_t entityID) {
        if (entityExists(entityID)) {
            entities.erase(std::remove(entities.begin(), entities.end(), entityID), entities.end());
            entityComponentMasks[entityID].reset();
            entityCount--;
        }
    }

    void EntityManager::addComponent(uint32_t entityID, ComponentType type) {
        assert(entityID < maxEntities);

        // Add ImageComponent if ModelComponent is being added
        if (type == ComponentType::Model && !hasComponent<ImageComponent>(entityID)) {
            std::cout << "Adding noTextureComponent to: " << entityID << std::endl;
            entityComponentMasks[entityID][static_cast<size_t>(ComponentType::Image)] = true;
            if (componentPools[static_cast<size_t>(ComponentType::Image)].size() <= entityID) {
                componentPools[static_cast<size_t>(ComponentType::Image)].resize(entityID + 1);
            }
            componentPools[static_cast<size_t>(ComponentType::Image)][entityID] = nullptr;
            // Add default noTexture
            this->setComponentData(entityID, noTextureComp);
        }
        

        // Add TransformComponent if PhysicsComponent is being added
        if (type == ComponentType::Physics && !hasComponent<TransformComponent>(entityID)) {
            entityComponentMasks[entityID][static_cast<size_t>(ComponentType::Transform)] = true;
            if (componentPools[static_cast<size_t>(ComponentType::Transform)].size() <= entityID) {
                componentPools[static_cast<size_t>(ComponentType::Transform)].resize(entityID + 1);
            }
            componentPools[static_cast<size_t>(ComponentType::Transform)][entityID] = nullptr;
        }

        entityComponentMasks[entityID][static_cast<size_t>(type)] = true;
        if (componentPools[static_cast<size_t>(type)].size() <= entityID) {
            componentPools[static_cast<size_t>(type)].resize(entityID + 1);
        }
        componentPools[static_cast<size_t>(type)][entityID] = nullptr;
    }

    void EntityManager::addComponents(uint32_t entityID, const std::vector<ComponentType>& componentTypes) {
        assert(entityID < maxEntities);
        for (const ComponentType type : componentTypes) {
            entityComponentMasks[entityID][static_cast<size_t>(type)] = true;
            if (componentPools[static_cast<size_t>(type)].size() <= entityID) {
                componentPools[static_cast<size_t>(type)].resize(entityID + 1);
            }
            componentPools[static_cast<size_t>(type)][entityID] = nullptr;
        }
    }

    void EntityManager::removeComponent(uint32_t entityID, ComponentType type) {
        assert(entityID < maxEntities);
        if (entityComponentMasks[entityID][static_cast<size_t>(type)]) {
            entityComponentMasks[entityID][static_cast<size_t>(type)] = false;

            if (entityID < componentPools[static_cast<size_t>(type)].size()) {
                // Clear the component data for the specified entity
                componentPools[static_cast<size_t>(type)][entityID].reset();
            }
        }
    }

    std::vector<uint32_t> EntityManager::getEntitiesWithComponent(ComponentType type) {
        std::vector<uint32_t> entitiesWithComponent;
        for (uint32_t entityID = 0; entityID < entityCount; entityID++) {
            if (entityComponentMasks[entityID][static_cast<size_t>(type)]) {
                entitiesWithComponent.push_back(entityID);
            }
        }
        return entitiesWithComponent;
    }

    uint32_t EntityManager::findAvailableEntityID() {
        for (uint32_t i = 0; i < maxEntities; i++) {
            if (!entityComponentMasks[i].any()) {
                return i;
            }
        }
        return 0;
    }
} //namespace engine
