#pragma once

#include "Buffer.hpp"
#include "Image.hpp"
#include "Descriptors.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace engine {

	struct BoundingBox {
		glm::vec3 min;
		glm::vec3 max;

		void scale(glm::vec3 scale) {
			min *= scale;
			max *= scale;
		}
	};

	struct BoundingSphere {
		glm::vec3 center;
		float radius;
	};

	class Model {
	public:

		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator ==(const Vertex& other) const {
				return position == other.position && color == other.color && normal == other.normal && 
					uv == other.uv;
			}
		};

		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filepath);
		};

		Model(Device& device, const Model::Builder& builder);
		~Model();

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;

		static std::unique_ptr<Model> createModelFromFile(
			Device& device, const std::string& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
		// void drawTexture(VkDescriptorSet set, DescriptorSetLayout& setLayout, DescriptorPool& pool);

		BoundingBox getBoundingBox() const { return m_bbox; };
		BoundingSphere getBoundingSphere() const { return m_bsphere; };
	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);
		BoundingBox createBoundingBox() const;
		BoundingSphere createBoundingSphere() const;
		Device& m_device;
		std::vector<Vertex> m_vertices{};
		std::vector<uint32_t> m_indices{};
		BoundingBox m_bbox;
		BoundingSphere m_bsphere;

		std::unique_ptr<Buffer> vertexBuffer;
		uint32_t vertexCount;

		std::unique_ptr<Buffer> indexBuffer;
		bool hasIndexBuffer = false;
		bool textureRendered = false;
		uint32_t indexCount;
	};
}
