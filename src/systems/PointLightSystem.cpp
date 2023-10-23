#include "PointLightSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <cassert>
#include <iostream>

namespace engine
{

    struct PointLightPushConstants
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(
        Device &device,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout) : m_device{device}
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem()
    {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("error while creating pipelineLayout");
        }
    }
    void PointLightSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<Pipeline>(
            m_device,
            "shaders/point_light.vert.spv",
            "shaders/point_light.frag.spv",
            pipelineConfig);
    }

    void PointLightSystem::update(FrameInfo &frameInfo, GlobalUbo &ubo)
    {
        auto rotateLight = glm::rotate(
            glm::mat4(1.f),
            frameInfo.frameTime,
            {0.f, -1.f, 0.f});
        int lightIndex = 0;
        for (auto &entityId : frameInfo.entityManager.getEntitiesWithComponent(ComponentType::PointLight))
        {
            auto transformComponent = frameInfo.entityManager.getComponentData<TransformComponent>(entityId);
            auto pointLightComponent = frameInfo.entityManager.getComponentData<PointLightComponent>(entityId);

            assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum");

            // Update
            transformComponent.translation = glm::vec3(rotateLight * glm::vec4(transformComponent.translation, 1.f));
            frameInfo.entityManager.setComponentData(entityId, transformComponent);
            ubo.pointLights[lightIndex].position = glm::vec4(transformComponent.translation, 1.f);
            ubo.pointLights[lightIndex].color = glm::vec4(pointLightComponent.color, pointLightComponent.lightIntensity);
            lightIndex++;
        }
        ubo.numLights = lightIndex;
    }

    void PointLightSystem::render(FrameInfo &frameInfo)
    {
        m_pipeline->bind(frameInfo.commandBuffer);
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        for (auto &entityId : frameInfo.entityManager.getEntitiesWithComponent(ComponentType::PointLight))
        {
            auto transformComponent = frameInfo.entityManager.getComponentData<TransformComponent>(entityId);
            auto pointLightComponent = frameInfo.entityManager.getComponentData<PointLightComponent>(entityId);

            PointLightPushConstants push{};
            push.position = glm::vec4(transformComponent.translation, 1.f);
            push.color = glm::vec4(pointLightComponent.color, pointLightComponent.lightIntensity);
            push.radius = transformComponent.scale.x;

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstants),
                &push);
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }
} // namespace engine
