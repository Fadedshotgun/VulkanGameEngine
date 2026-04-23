#include "defaultRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#define RENDER_DISTANCE 100

namespace v
{
    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.f};
    };

    DefaultRenderSystem::DefaultRenderSystem(vDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout, VkDescriptorSet defaultTextureSet) : device(device), renderPass(renderPass), defaultTextureSet(defaultTextureSet)
    {
        createPipelineLayout(globalSetLayout, textureSetLayout);
        createPipeline(this->renderPass, 0);
    }

    DefaultRenderSystem::~DefaultRenderSystem()
    {
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void DefaultRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout, textureSetLayout};

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

    void DefaultRenderSystem::createPipeline(VkRenderPass renderPass, int rasterMode)
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
        
        pipeline = std::make_unique<vPipeline>(device, pipelineConfig,  std::string(PROJECT_ROOT) + "shaders/default.vert.spv", std::string(PROJECT_ROOT) + "shaders/default.frag.spv");
    }

    void DefaultRenderSystem::renderGameObjects(FrameInfo &frameInfo)
    {
        if (currentRasterMode != frameInfo.rasterMode)
        {
            currentRasterMode = frameInfo.rasterMode;
            createPipeline(renderPass, currentRasterMode);
        }

        glm::mat4 projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

        pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

        vModel *lastBoundModel = nullptr;

        frameInfo.entityStore.forEach<ecs::MeshRendererComponent, ecs::TransformComponent>([&](ecs::MeshRendererComponent &mesh, ecs::TransformComponent &transform)
        {
            if (!mesh.active) return;
            if (mesh.model == nullptr) return;

         SimplePushConstantData push{};
            push.modelMatrix = transform.mat4();

            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

            if (mesh.model.get() != lastBoundModel)
            {
                mesh.model->bind(frameInfo.commandBuffer);

                VkDescriptorSet textureSet = mesh.model->hasTextureDescriptorSet()
                    ? mesh.model->getTextureDescriptorSet()
                    : defaultTextureSet;
                vkCmdBindDescriptorSets(
                    frameInfo.commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout,
                    1,
                    1,
                    &textureSet,
                    0,
                    nullptr);

                lastBoundModel = mesh.model.get();
            }
            mesh.model->draw(frameInfo.commandBuffer);
        });

        // std::cout << "objects culled: " << culled << "/4096\n";
    }
}