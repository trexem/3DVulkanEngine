#pragma once

#include "Renderer.hpp"
#include "KeyboardMovementController.hpp"
#include "Descriptors.hpp"
#include "EntityManager.hpp"

namespace engine {

	static constexpr size_t MAX_ENTITIES = 500;
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
		std::vector<std::shared_ptr<Image>> images;

		std::unique_ptr<DescriptorPool> globalPool{};
		std::unique_ptr<DescriptorPool> texturePool{};
		EntityManager entityManager = EntityManager(MAX_ENTITIES);
	};
} // namespace engine