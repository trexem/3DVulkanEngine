#pragma once

#include "GameObject.hpp"
#include "Window.hpp"

namespace engine {

	class KeyboardMovementController {
	public:
		struct KeyMappings
		{
			int moveLeft = SDL_SCANCODE_A; 
			int moveRight = SDL_SCANCODE_D;
			int moveForward = SDL_SCANCODE_W;
			int moveBackward = SDL_SCANCODE_S;
			int moveUp = SDL_SCANCODE_SPACE;
			int moveDown = SDL_SCANCODE_LSHIFT;
			int lookLeft = SDL_SCANCODE_LEFT;
			int lookRight = SDL_SCANCODE_RIGHT;
			int lookUp= SDL_SCANCODE_UP;
			int lookDown = SDL_SCANCODE_DOWN;
		};

		void moveInPlaneXZ(const Uint8* t_current_key_states, float dt, GameObject& gameObject);

		KeyMappings keys{};
		float moveSpeed{ 3.f };
		float lookSpeed{ 1.5f };
	};
} // engine namespace
