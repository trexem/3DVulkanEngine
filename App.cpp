#include "App.hpp"

#include "SimpleRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <cassert>

namespace engine {

    App::App() {
        loadGameObjects();
    }

    App::~App() {}

	void App::run() {
        SimpleRenderSystem simpleRenderSystem(m_device, renderer.getSwapChainRenderPass());
        Camera camera{};
        /*camera.setViewDirection(glm::vec3(0.f), glm::vec3(.5f, .0f, 1.f));*/
        camera.setViewTarget(glm::vec3(-1.f,-20.f,2.f), glm::vec3(.0f, .0f, 1.5f));
        while (m_window.m_stillRunning) {
            handleSDLEvents();
            float aspect = renderer.getAspectRatio();
            /*camera.setOrthographicProjection(
                -aspect, aspect, 
                -1, 1, 
                -1, 1
            );*/
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
                case SDLK_q:
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

    std::unique_ptr<Model> createCubeModel(Device& device, glm::vec3 offset) {
        std::vector<Model::Vertex> vertices{

            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

            // right face (yellow)
            {{.5f, -.5f, -.5f}, {1.f, .8f, .0f}},
            {{.5f, .5f, .5f}, {1.f, .8f, .0f}},
            {{.5f, -.5f, .5f}, {1.f, .8f, .0f}},
            {{.5f, -.5f, -.5f}, {1.f, .8f, .0f}},
            {{.5f, .5f, -.5f}, {1.f, .8f, .0f}},
            {{.5f, .5f, .5f}, {1.f, .8f, .0f}},

            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {1.f, .34f, .0f}},
            {{.5f, -.5f, .5f}, {1.f, .34f, .0f}},
            {{-.5f, -.5f, .5f}, {1.f, .34f, .0f}},
            {{-.5f, -.5f, -.5f}, {1.f, .34f, .0f}},
            {{.5f, -.5f, -.5f}, {1.f, .34f, .0f}},
            {{.5f, -.5f, .5f}, {1.f, .34f, .0f}},

            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .0f, .0f}},
            {{.5f, .5f, .5f}, {.8f, .0f, .0f}},
            {{-.5f, .5f, .5f}, {.8f, .0f, .0f}},
            {{-.5f, .5f, -.5f}, {.8f, .0f, .0f}},
            {{.5f, .5f, -.5f}, {.8f, .0f, .0f}},
            {{.5f, .5f, .5f}, {.8f, .0f, .0f}},

            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.0f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.0f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.0f, .1f, .8f}},
            {{-.5f, -.5f, 0.5f}, {.0f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.0f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.0f, .1f, .8f}},

            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.0f, .8f, .0f}},
            {{.5f, .5f, -0.5f}, {.0f, .8f, .0f}},
            {{-.5f, .5f, -0.5f}, {.0f, .8f, .0f}},
            {{-.5f, -.5f, -0.5f}, {.0f, .8f, .0f}},
            {{.5f, -.5f, -0.5f}, {.0f, .8f, .0f}},
            {{.5f, .5f, -0.5f}, {.0f, .8f, .0f}},

        };
        for (auto& v : vertices) {
            v.position += offset;
        }
        return std::make_unique<Model>(device, vertices);
    }

    void App::loadGameObjects() {
        std::shared_ptr<Model> model = createCubeModel(m_device, { .0f,.0f,.0f });

        auto cube = GameObject::createGameObject();
        cube.model = model;
        cube.transform.translation = { .0f,.0f,1.5f };
        cube.transform.scale = { .5f, .5f, .5f };
        m_gameObjects.push_back(std::move(cube));
    }
} // namespace engine
