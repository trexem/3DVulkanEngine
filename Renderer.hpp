#pragma once

#include "Device.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"

#include <cassert>

namespace engine {

	class Renderer {
	public:

		Renderer(Window& window, Device& device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		VkRenderPass getSwapChainRenderPass() const { return m_swapChain->getRenderPass(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return m_commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const { 
			assert(isFrameStarted && "Cannot get frameIndex when frame not in progress");
			return currentFrameIndex; 
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		Window& m_window;
		Device& m_device;
		std::unique_ptr<SwapChain> m_swapChain;
		std::vector<VkCommandBuffer> m_commandBuffers;
		uint32_t currentImageIndex{ 0 };
		int currentFrameIndex{ 0 };
		bool isFrameStarted{ false };
	};
} // namespace engine
