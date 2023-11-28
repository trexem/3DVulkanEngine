#include "App.hpp"

#include "systems/SimpleRenderSystem.hpp"
#include "KeyboardMovementController.hpp"
#include "systems/PointLightSystem.hpp"

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
                    .addPoolSize(
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
                        entityManager.getEntitiesWithComponent(ComponentType::Image).size())
                    .build();

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

        auto textureSetLayout = DescriptorSetLayout::Builder(m_device)
                                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                .build();

        
        std::vector<VkDescriptorSet> globalTextureDescriptorSets(entityManager.getEntitiesWithComponent(ComponentType::Image).size());
        int textureIterator = 0;
        for (const uint32_t entityID : entityManager.getEntitiesWithComponent(ComponentType::Image)) {
        std::vector<VkDescriptorImageInfo> imageInfos; 
            if (entityManager.entityExists(entityID)) {
                ImageComponent imageComponent = entityManager.getComponentData<ImageComponent>(entityID);
                for (const auto& texInfo : imageComponent.textureInfo) {
                    if (texInfo.imageView == VK_NULL_HANDLE || texInfo.descriptorInfo.imageView == VK_NULL_HANDLE)  {
                        throw std::runtime_error("Invalid VkImageView handle in TextureInfo!");
                    }
                    imageInfos.emplace_back(texInfo.descriptorInfo);
                }
                DescriptorWriter(*textureSetLayout, *texturePool)
                    .writeImage(0,imageInfos.data(), imageInfos.size())
                    .build(globalTextureDescriptorSets.at(textureIterator));

                imageComponent.pDescriptorSet = &globalTextureDescriptorSets.at(textureIterator);
                entityManager.setComponentData<ImageComponent>(entityID, imageComponent);
            }
            textureIterator++;
        }

        SimpleRenderSystem simpleRenderSystem{
            m_device, renderer.getSwapChainRenderPass(), 
            globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout()};

        PointLightSystem pointLightSysyem{
            m_device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        Camera camera{};

        TransformComponent viewerObject {};
        viewerObject.translation.z = -2.5f;

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
                    entityManager};
                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSysyem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

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

    void App::loadGameObjects()
    {
        uint32_t flatVase = entityManager.createEntity();
        std::shared_ptr<Model> model =
            Model::createModelFromFile(m_device, "models/Quad.obj");
        
        
        entityManager.addComponent(flatVase, ComponentType::Model);
        ModelComponent flatVaseModel;
        flatVaseModel.model = model;
        entityManager.setComponentData(flatVase, flatVaseModel);
        
        std::shared_ptr<Image> image = std::make_shared<Image>(m_device, "textures/texture.jpg");
        ImageComponent flatVaseTexture;
        flatVaseTexture.textureInfo.push_back(image->textureInfo());
        entityManager.addComponent(flatVase, ComponentType::Image);
        entityManager.setComponentData(flatVase, flatVaseTexture);

        images.push_back(image);

        entityManager.addComponent(flatVase, ComponentType::Transform);
        TransformComponent flatVaseTransform{};
        flatVaseTransform.translation = {-.75f, .5f, 0.f};
        flatVaseTransform.scale = {3.0f, 1.5, 3.0};
        entityManager.setComponentData(flatVase, flatVaseTransform);

        // model = Model::createModelFromFile(m_device, "models/shiptest.obj");
        // model->loadTexture("textures/texture.jpg");

        // uint32_t smoothVase = entityManager.createEntity();
        
        // entityManager.addComponent(smoothVase, ComponentType::Model);
        // ModelComponent smoothVaseModel;
        // smoothVaseModel.model = model;
        // entityManager.setComponentData(smoothVase, smoothVaseModel);

        // entityManager.addComponent(smoothVase, ComponentType::Transform);
        // TransformComponent smoothVaseTransform{};
        // smoothVaseTransform.translation = {.5f, .5f, .0f};
        // smoothVaseTransform.scale = {.02f, .02f, .02f};
        // entityManager.setComponentData(smoothVase, smoothVaseTransform);

        // model =
        //     Model::createModelFromFile(m_device, "models/Quad.obj");
        // uint32_t floor = entityManager.createEntity();
        
        // entityManager.addComponent(floor, ComponentType::Model);
        // ModelComponent floorModel;
        // floorModel.model = model;
        // entityManager.setComponentData(floor, floorModel);

        // entityManager.addComponent(floor, ComponentType::Transform);
        // TransformComponent floorTransform{};
        // floorTransform.translation = {0.f, .5f, 0.f};
        // floorTransform.scale = {5.0f, 1.0, 5.0};
        // entityManager.setComponentData(floor, floorTransform);

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
