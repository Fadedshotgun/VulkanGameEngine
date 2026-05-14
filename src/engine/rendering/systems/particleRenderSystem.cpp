#include "particleRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#define RENDER_DISTANCE 100

namespace v
{
    struct ParticlePushConstantData
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float size{};
    };

    ParticleRenderSystem::ParticleRenderSystem(vDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device(device), renderPass(renderPass)
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(this->renderPass, 0);
    }

    ParticleRenderSystem::~ParticleRenderSystem()
    {
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void ParticleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(ParticlePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout");
        }
    }

    void ParticleRenderSystem::createPipeline(VkRenderPass renderPass, int rasterMode)
    {
        assert(pipelineLayout != nullptr && "Pipeline layout must be created before pipeline");

        PipelineConfigInfo pipelineConfig{};

        vPipeline::defaultPipelineConfigInfo(pipelineConfig);

        switch (rasterMode)
        {
        case 0:
            pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
            break;
        case 1:
            pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
            break;
        case 2:
            pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_POINT;
            break;
        default:
            pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
            break;
        }

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;

        pipeline = std::make_unique<vPipeline>(device, pipelineConfig, std::string(PROJECT_ROOT) + "shaders/particle.vert.spv", std::string(PROJECT_ROOT) + "shaders/particle.frag.spv");
    }

    void ParticleRenderSystem::render(FrameInfo &frameInfo)
    {
        if (currentRasterMode != frameInfo.rasterMode)
        {
            currentRasterMode = frameInfo.rasterMode;
            createPipeline(renderPass, currentRasterMode);
        }

        glm::mat4 projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

        pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

        frameInfo.entityRegistry.forEach<ecs::ParticleEmitterComponent, ecs::TransformComponent>(
            [&](ecs::ParticleEmitterComponent &emitter, ecs::TransformComponent &transform)
            {
                for (auto particle : emitter.particles)
                {
                    ParticlePushConstantData push{};
                    push.position = glm::vec4(particle.position, 1.f);
                    push.color = glm::vec4(particle.color, 1);
                    push.size = particle.size / 100;

                    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ParticlePushConstantData), &push);

                    vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
                }
            });
    }
}