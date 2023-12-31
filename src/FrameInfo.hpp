#pragma once

#include "Camera.hpp"
#include "EntityManager.hpp"

#include <vulkan/vulkan.h>

namespace engine {
	#define MAX_LIGHTS 10

	struct PointLight {
		glm::vec4 position{};
		glm::vec4 color{};
	};

	struct GlobalUbo {
        glm::mat4 projection{ 1.f };
        glm::mat4 view{ 1.f };
		glm::mat4 inverseView{ 1.f };
        glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f ,.02f };
        PointLight pointLights[MAX_LIGHTS];
		int numLights;
    };

	struct TextureData {
		int textureIndex;
	};

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		Camera& camera;
		VkDescriptorSet globalDescriptorSet;
		EntityManager& entityManager;
		std::vector<std::shared_ptr<Buffer>> textureBuffers;
	};
} //namespace engine
