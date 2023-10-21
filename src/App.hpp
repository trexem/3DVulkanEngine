#pragma once

#include "Renderer.hpp"
#include "GameObject.hpp"
#include "KeyboardMovementController.hpp"
#include "Descriptors.hpp"

namespace engine {

	class App {
	public:
		static constexpr int WIDTH = 1920;
		static constexpr int HEIGHT = 1200;

		App();
		~App();

		App(const App&) = delete;
		App& operator=(const App&) = delete;

		void run();
		void handleSDLEvents();

	private:
		void loadGameObjects();

		Window m_window{ WIDTH, HEIGHT, "Hello Vulkan!" };
		Device m_device{ m_window };
		Renderer renderer{ m_window, m_device };

		std::unique_ptr<DescriptorPool> globalPool{};
		GameObject::Map m_gameObjects;
	};
} // namespace engine