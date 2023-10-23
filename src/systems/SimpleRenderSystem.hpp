#pragma once

#include "Pipeline.hpp"
#include "FrameInfo.hpp"

namespace engine {

	class SimpleRenderSystem {
	public:

		SimpleRenderSystem(
			Device& device, 
			VkRenderPass renderPass,
			VkDescriptorSetLayout globalSetLayout
		);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);
	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		Device& m_device;
		std::unique_ptr<Pipeline> m_pipeline;
		VkPipelineLayout m_pipelineLayout;
	};
} // namespace engine