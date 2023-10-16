#include "KeyboardMovementController.hpp"

namespace engine {
	void KeyboardMovementController::moveInPlaneXZ(
		const Uint8* t_current_key_states, float dt, GameObject& gameObject
	) {
		glm::vec3 rotate{ 0 };
		if (t_current_key_states[keys.lookRight]) rotate.y += 1.f;
		if (t_current_key_states[keys.lookLeft]) rotate.y -= 1.f;
		if (t_current_key_states[keys.lookUp]) rotate.x += 1.f;
		if (t_current_key_states[keys.lookDown]) rotate.x -= 1.f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}
		gameObject.transform.rotation.x = 
			glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
		gameObject.transform.rotation.y =
			glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

		float yaw = gameObject.transform.rotation.y;
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		glm::vec3 moveDir{ 0 };

		if (t_current_key_states[keys.moveForward]) {
			moveDir += forwardDir;
		}
		if (t_current_key_states[keys.moveBackward]) moveDir -= forwardDir;
		if (t_current_key_states[keys.moveLeft]) moveDir -= rightDir;
		if (t_current_key_states[keys.moveRight]) moveDir += rightDir;
		if (t_current_key_states[keys.moveUp]) moveDir += upDir;
		if (t_current_key_states[keys.moveDown]) moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}
	}
}