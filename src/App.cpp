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
                         .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                         .build();
        loadGameObjects();
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

        SimpleRenderSystem simpleRenderSystem{
            m_device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};

        PointLightSystem pointLightSysyem{
            m_device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        Camera camera{};

        auto viewerObject = GameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;

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
                viewerObject.transform.translation, viewerObject.transform.rotation);

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
                    m_gameObjects};
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
        std::shared_ptr<Model> model =
            Model::createModelFromFile(m_device, "models/flat_vase.obj");
        auto flatVase = GameObject::createGameObject();
        flatVase.model = model;
        flatVase.transform.translation = {-.5f, .5f, 0.f};
        flatVase.transform.scale = {3.0f, 1.5, 3.0};
        m_gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        model =
            Model::createModelFromFile(m_device, "models/smooth_vase.obj");
        auto smoothVase = GameObject::createGameObject();
        smoothVase.model = model;
        smoothVase.transform.translation = {.5f, .5f, .0f};
        smoothVase.transform.scale = {3.0f, 1.5, 3.0};
        m_gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        model =
            Model::createModelFromFile(m_device, "models/Quad.obj");
        auto floor = GameObject::createGameObject();
        floor.model = model;
        floor.transform.translation = {0.f, .5f, 0.f};
        floor.transform.scale = {5.0f, 1.0, 5.0};
        m_gameObjects.emplace(floor.getId(), std::move(floor));

        std::vector<glm::vec3> lightColors{
            {.8f, 0.f, 0.f},
            {0.f, 0.f, .8f},
            {0.f, .8f, 0.f},
            {.8f, .8f, 0.f},
            {0.f, .8f, .8f},
            {.8f, 0.f, 0.8f},
        };
        for (int i = 0; i < lightColors.size(); i++)
        {
            auto pointLight = GameObject::makePointLight(1.f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            m_gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }
} // namespace engine
