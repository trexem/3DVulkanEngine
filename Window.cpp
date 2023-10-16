#include "Window.hpp"

#include <iostream>
#include <stdexcept>

namespace engine {
    Window::Window(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

    Window::~Window() {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

	void Window::initWindow() {
        // Create an SDL window that supports Vulkan rendering.
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cout << "Could not initialize SDL." << std::endl;
        }
        window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, width, height,
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
        if (window == NULL) {
            std::cout << "Could not create SDL window." << std::endl;
        }
        
	}

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
        if (!SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance), surface)) {
            throw std::runtime_error("failed to create window surface");
            return;
        }
    }

    void Window::recreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
        vkDestroySurfaceKHR(instance, *surface, nullptr);
        if (!SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance), surface)) {
            throw std::runtime_error("failed to create window surface");
            return;
        }
    }

    void Window::framebufferResizeCallback() {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        framebufferResized = true;
        width = w;
        height = h;
    }
} // namespace engine
