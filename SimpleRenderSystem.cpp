#include "SimpleRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <cassert>

namespace engine {

    struct SimplePushConstantData
    {
        glm::mat4 transform{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
    };

    SimpleRenderSystem::SimpleRenderSystem(Device& device, VkRenderPass renderPass) : 
        m_device{ device } {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("error while creating pipelineLayout");
        }
    }
    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<Pipeline>(
            m_device,
            "E:/Code/VulkanSDL/shaders/simple_shader.vert.spv",
            "E:/Code/VulkanSDL/shaders/simple_shader.frag.spv",
            pipelineConfig);
    }

    void SimpleRenderSystem::renderGameObjects(
        VkCommandBuffer commandBuffer,
        std::vector<GameObject>& gameObjects,
        const Camera& camera
    ) {
        m_pipeline->bind(commandBuffer);

        auto projectionView = camera.getProjection() * camera.getView();

        for (auto& obj : gameObjects) {

            SimplePushConstantData push{};
            push.transform = projectionView * obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(
                commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push
            );
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }
} // namespace engine
