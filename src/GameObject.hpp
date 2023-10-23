#pragma once

#include "Model.hpp"
#include "Components.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace engine {

	class GameObject {
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, GameObject>;

		static GameObject createGameObject() {
			static id_t currentId = 0;
			return GameObject(currentId++);
		}

		static GameObject makePointLight(
			float intensity = 3.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f)
		);

		const id_t getId() { return id; };

		GameObject(const GameObject&) = delete;
		GameObject& operator=(const GameObject&) = delete;
		GameObject(GameObject&&) = default;
		GameObject& operator=(GameObject&&) = default;

		glm::vec3 color{};
		TransformComponent transform{};

		std::shared_ptr<Model> model{};
		std::unique_ptr<PointLightComponent> pointLight = nullptr;

	private:
		GameObject(id_t objId) : id{ objId } {};

		id_t id;
	};
}
