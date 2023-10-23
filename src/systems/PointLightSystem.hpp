#pragma once

#include "Pipeline.hpp"
#include "FrameInfo.hpp"

namespace engine {

	class PointLightSystem {
	public:

		PointLightSystem(
			Device& device, 
			VkRenderPass renderPass,
			VkDescriptorSetLayout globalSetLayout
		);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo &ubo);
		void render(FrameInfo& frameInfo);
	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		Device& m_device;
		std::unique_ptr<Pipeline> m_pipeline;
		VkPipelineLayout m_pipelineLayout;
	};
} // namespace engine