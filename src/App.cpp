#include "App.hpp"

#include "SimpleRenderSystem.hpp"
#include "KeyboardMovementController.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <chrono>
#include <cassert>

namespace engine {

    App::App() {
        loadGameObjects();
    }

    App::~App() {}

	void App::run() {
        SimpleRenderSystem simpleRenderSystem(m_device, renderer.getSwapChainRenderPass());
        Camera camera{};

        auto viewerObject = GameObject::createGameObject();
        KeyboardMovementController cameraController {};
        auto currentTime = std::chrono::high_resolution_clock::now();
        while (m_window.m_stillRunning) {
            KeyboardMovementController::KeyMappings kMap{};
            handleSDLEvents();
            const Uint8* current_key_states = SDL_GetKeyboardState(NULL);

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = 
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(
                current_key_states, frameTime, viewerObject
            );
            camera.setViewYXZ(
                viewerObject.transform.translation, viewerObject.transform.rotation
            );


            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 100.f);
            if (auto commandBuffer = renderer.beginFrame()) {
                renderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects, camera);
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(m_device.device());
	}

    void App::handleSDLEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            switch (event.type) {

            case SDL_QUIT:
                m_window.m_stillRunning = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    m_window.m_stillRunning = false;
                    break;

                default:
                    //Do nothing
                    break;
                }

                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED &&
                    event.window.windowID == SDL_GetWindowID(m_window.window)) {
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
        std::shared_ptr<Model> model = 
            Model::createModelFromFile(m_device, "models/flat_vase.obj");
        auto flatVase = GameObject::createGameObject();
        flatVase.model = model;
        flatVase.transform.translation = { .0f,.5f,1.5f };
        flatVase.transform.scale = { 3.0f, 1.5, 3.0 };
        m_gameObjects.push_back(std::move(flatVase));

        model =
            Model::createModelFromFile(m_device, "models/smooth_vase.obj");
        auto smoothVase = GameObject::createGameObject();
        smoothVase.model = model;
        smoothVase.transform.translation = { .0f,.5f,.5f };
        smoothVase.transform.scale = { 3.0f, 1.5, 3.0 };
        m_gameObjects.push_back(std::move(smoothVase));
    }
} // namespace engine
