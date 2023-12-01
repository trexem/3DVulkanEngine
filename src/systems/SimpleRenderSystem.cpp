#include "SimpleRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <cassert>
#include <iostream>

namespace engine {

    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
    };

    SimpleRenderSystem::SimpleRenderSystem(
        Device& device,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout,
        VkDescriptorSetLayout textureSetLayout
    ) : m_device{ device } {
        createPipelineLayout(globalSetLayout, textureSetLayout);
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout(
        VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout, textureSetLayout };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
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
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineConfig);
    }

    void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo) {
        m_pipeline->bind(frameInfo.commandBuffer);
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );

        auto projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

        for (const uint32_t entityID : frameInfo.entityManager.getEntitiesWithComponent(ComponentType::Model)) {
            if (frameInfo.entityManager.entityExists(entityID)) {
                ModelComponent modelComponent = 
                    frameInfo.entityManager.getComponentData<ModelComponent>(entityID);
                TransformComponent transformComponent = 
                    frameInfo.entityManager.getComponentData<TransformComponent>(entityID);

                SimplePushConstantData push{};
                push.modelMatrix = transformComponent.mat4();
                push.normalMatrix = transformComponent.normalMatrix();

                vkCmdPushConstants(
                    frameInfo.commandBuffer,
                    m_pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(SimplePushConstantData),
                    &push
                );
                if (frameInfo.entityManager.hasComponent<ImageComponent>(entityID)){
                    int i = 0;
                    ImageComponent imageComponent = 
                    frameInfo.entityManager.getComponentData<ImageComponent>(entityID);
                    for (const auto descriptor : imageComponent.pDescriptorSet) {
                        TextureData tex{};
                        tex.textureIndex = imageComponent.textureInfo.at(i).textureIndex;
                        frameInfo.textureBuffers[imageComponent.textureBufferIndex.at(i)]->writeToBuffer(&tex);
                        frameInfo.textureBuffers[imageComponent.textureBufferIndex.at(i)]->flush();
                        vkCmdBindDescriptorSets(
                            frameInfo.commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipelineLayout,
                            1,
                            1,
                            descriptor,
                            0,
                            nullptr
                        );
                        modelComponent.model->bind(frameInfo.commandBuffer);
                        modelComponent.model->draw(frameInfo.commandBuffer);
                        i++;
                    }
                } else {
                    modelComponent.model->bind(frameInfo.commandBuffer);
                    modelComponent.model->draw(frameInfo.commandBuffer);
                }
            }
        }
    }
} // namespace engine
