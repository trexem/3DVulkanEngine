#include "App.hpp"

#include "systems/SimpleRenderSystem.hpp"
#include "KeyboardMovementController.hpp"
#include "systems/PointLightSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/CollisionSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <chrono>
#include <cassert>
#include <numeric>

namespace engine
{
    App::App()
    {
        globalPool = DescriptorPool::Builder(m_device)
                    .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2)
                    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                    .build();
        loadGameObjects();
        texturePool = DescriptorPool::Builder(m_device)
                    .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2)
                    .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, images.size())
                    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, images.size())
                    .build();
        std::cout << "Succesfully createdTexturePools with imageSize: " << images.size() << std::endl;
    }

    App::~App() {}

    void App::run()
    {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<Buffer>(
                m_device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = DescriptorSetLayout::Builder(m_device)
                                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                .build();
        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }
        
        uint16_t textureNum = images.size();
        std::vector<std::shared_ptr<Buffer>> textureBuffers(textureNum);
        for (int i = 0; i < textureBuffers.size(); i++) {
            textureBuffers[i] = std::make_shared<Buffer>(
                m_device,
                sizeof(TextureData),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            std::cout << "Succesfully maped textureBuffer number: " << i << std::endl;
            textureBuffers[i]->map();
        }

        auto textureSetLayout = DescriptorSetLayout::Builder(m_device)
                                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                .build();

        
        std::vector<VkDescriptorSet> textureDescriptorSets(images.size());
        std::vector<bool> imageAdded(images.size());
        for (const uint32_t entityID : entityManager.getEntitiesWithComponent(ComponentType::Image)) {
            if (entityManager.entityExists(entityID)) {
                std::cout << "Entity id: " << entityID << " has ";
                ImageComponent imageComponent = entityManager.getComponentData<ImageComponent>(entityID);
                for (auto& imageIndex : imageComponent.imagesIndex) {
                    auto texInfo = images.at(imageIndex)->textureInfo();
                    auto& buffer = textureBuffers[imageIndex];
                    if (texInfo.imageView == VK_NULL_HANDLE || texInfo.descriptorInfo.imageView == VK_NULL_HANDLE)  {
                        throw std::runtime_error("Invalid VkImageView handle in TextureInfo!");
                    }
                    std::cout << "image: " << imageIndex << " ";
                    auto bufferInfo = buffer->descriptorInfo();
                    DescriptorWriter(*textureSetLayout, *texturePool)
                        .writeImage(0,&texInfo.descriptorInfo)
                        .writeBuffer(1,&bufferInfo)
                        .build(textureDescriptorSets.at(imageIndex));
                    imageComponent.pDescriptorSet.emplace_back(&textureDescriptorSets.at(imageIndex));
                    imageComponent.textureBufferIndex.emplace_back(imageIndex);
                    imageAdded.at(imageIndex) = true;
                }
                std::cout << std::endl;
                entityManager.setComponentData<ImageComponent>(entityID, imageComponent);
            }
        }

        std::cout << "Succesfully created descriptorSets" << std::endl;

        SimpleRenderSystem simpleRenderSystem{
            m_device, renderer.getSwapChainRenderPass(), 
            globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout()};
        
        std::cout << "Succesfully initialized simpleRenderSystem" << std::endl;

        PointLightSystem pointLightSysyem{
            m_device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};

        PhysicsSystem physicsSystem;
        CollisionSystem collisionSystem;
        Camera camera{};

        TransformComponent viewerObject {};
        viewerObject.translation.z = -5.5f;

        KeyboardMovementController cameraController{};
        auto currentTime = std::chrono::high_resolution_clock::now();
        while (m_window.m_stillRunning)
        {
            KeyboardMovementController::KeyMappings kMap{};
            handleSDLEvents();
            const Uint8 *current_key_states = SDL_GetKeyboardState(NULL);

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(
                current_key_states, frameTime, viewerObject);
            camera.setViewYXZ(
                viewerObject.translation, viewerObject.rotation);

            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 100.f);
            if (auto commandBuffer = renderer.beginFrame())
            {
                int frameIndex = renderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    entityManager,
                    textureBuffers};
                
                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSysyem.update(frameInfo, ubo);
                collisionSystem.update(frameInfo);
                physicsSystem.update(frameInfo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                std::cout << "Succesfully updated systems and uboBuffer" << std::endl;

                // render
                renderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSysyem.render(frameInfo);
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(m_device.device());
    }

    void App::handleSDLEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {

            switch (event.type)
            {

            case SDL_QUIT:
                m_window.m_stillRunning = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    m_window.m_stillRunning = false;
                    break;

                default:
                    // Do nothing
                    break;
                }

                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED &&
                    event.window.windowID == SDL_GetWindowID(m_window.window))
                {
                    m_window.framebufferResizeCallback();
                    m_device.recreateSurface();
                }

            default:
                // Do nothing.
                break;
            }
        }
    }

    void App::loadGameObjects() {
        images.push_back(entityManager.noTexture);
        //****************** CUBE ***********************
        uint32_t cube = entityManager.createEntity();
        std::shared_ptr<Model> model = Model::createModelFromFile(m_device, "models/cube.obj");
        std::cout << "Cube has entityID: " << cube << std::endl;
        
        
        ImageComponent cubeTexture;
        std::shared_ptr<Image> image = std::make_shared<Image>(m_device, "textures/texture.jpg", 0);
        images.push_back(image);
        std::shared_ptr<Image> bridg4 = std::make_shared<Image>(m_device, "textures/bridge4.jpg", 1);
        images.push_back(bridg4);
        cubeTexture.imagesIndex.push_back(1);
        cubeTexture.textureInfo.push_back(image->textureInfo());
        cubeTexture.imagesIndex.push_back(2);
        cubeTexture.textureInfo.push_back(bridg4->textureInfo());
        entityManager.addComponent(cube, ComponentType::Image);
        entityManager.setComponentData(cube, cubeTexture);
        entityManager.addComponent(cube, ComponentType::Model);
        ModelComponent cubeModel;
        cubeModel.model = model;
        entityManager.setComponentData(cube, cubeModel);
        entityManager.addComponent(cube, ComponentType::Transform);
        TransformComponent cubeTransform{};
        cubeTransform.translation = {-.75f, .5f, 0.f};
        cubeTransform.scale = {1.0f, 1.0f, 1.0f};
        entityManager.setComponentData(cube, cubeTransform);

        //****************** SHIP ***********************
        model = Model::createModelFromFile(m_device, "models/shiptest.obj");
        uint32_t ship = entityManager.createEntity();
        std::cout << "Ship has entityID: " << ship << std::endl;
        entityManager.addComponent(ship, ComponentType::Model);
        ModelComponent shipModel;
        shipModel.model = model;
        entityManager.setComponentData(ship, shipModel);
        entityManager.addComponent(ship, ComponentType::Transform);
        TransformComponent shipTransform{};
        shipTransform.translation = {.5f, -5.5f, .0f};
        shipTransform.scale = {.02f, .02f, .02f};
        entityManager.addComponent(ship, ComponentType::Physics);
        PhysicsComponent physComp{};
        physComp.velocity = {.0f, 0.0f, 0.0f};
        physComp.acceleration = {.0f, .0f, -1.5f };
        physComp.hasGravity = true;
        physComp.coefRes = 0.4f;
        entityManager.setComponentData(ship, physComp);
        entityManager.setComponentData(ship, shipTransform);

        //****************** FLOOR ***********************
        model = Model::createModelFromFile(m_device, "models/Quad.obj");
        uint32_t floor = entityManager.createEntity();
        std::cout << "Floor has entityID: " << floor << std::endl;
        entityManager.addComponent(floor, ComponentType::Model);
        ModelComponent floorModel;
        floorModel.model = model;
        entityManager.setComponentData(floor, floorModel);

        entityManager.addComponent(floor, ComponentType::Transform);
        TransformComponent floorTransform{};
        floorTransform.translation = {0.f, 1.5f, 0.f};
        floorTransform.scale = {5.0f, 1.0, 5.0};

        entityManager.addComponent(floor, ComponentType::Physics);
        PhysicsComponent floorPhysComp{};
        floorPhysComp.movable = false;
        floorPhysComp.coefRes = 1.0f;
        floorPhysComp.hasGravity = false;
        entityManager.setComponentData(floor, floorPhysComp);
        entityManager.setComponentData(floor, floorTransform);

        std::vector<glm::vec3> lightColors{
            {.8f, 0.f, 0.f},
            {0.f, 0.f, .8f},
            {0.f, .8f, 0.f},
            {.8f, .8f, 0.f},
            {0.f, .8f, .8f},
            {.8f, 0.f, 0.8f},
        };
        std::vector<ComponentType> comps {
            ComponentType::PointLight,
            ComponentType::Transform,
        };
        for (int i = 0; i < lightColors.size(); i++)
        {
            uint32_t plId = entityManager.createEntity();
            entityManager.addComponents(plId, comps);
            PointLightComponent pointLight{};
            TransformComponent plTComp{};
            pointLight.color = lightColors[i];
            pointLight.lightIntensity = 5.5f;
            auto rotateLight = glm::rotate(
                glm::mat4(5.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                {0.f, -1.f, 0.f});
            plTComp.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            plTComp.scale.x = .1f;
            entityManager.setComponentData(plId, pointLight);
            entityManager.setComponentData(plId, plTComp);
        }
    }
} // namespace engine
