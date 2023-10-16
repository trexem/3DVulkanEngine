#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#include <string>

namespace engine {
	class Window
	{
	public:
		Window(int w, int h, std::string name);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		VkExtent2D getExtent() { 
			return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; 
		};
		bool wasWindowResized() { return framebufferResized; };
		void resetWindowResizedFlag() { framebufferResized = false; };

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		void recreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		bool m_stillRunning{ true };
		SDL_Window* window;
		void framebufferResizeCallback();
	private:
		void initWindow();
		int width;
		int height;
		bool framebufferResized = false;
		std::string windowName;
		
	};
} //namespace engine
