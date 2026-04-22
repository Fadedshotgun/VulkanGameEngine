#include "pointLightRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <map>

#define RENDER_DISTANCE 100

namespace v
{
    struct PointLightPushConstants
    {
        glm::vec4 position{};
        glm::vec4 color{}; // r, g, b, intensity
        float radius{1.f};
    };

    PointLightRenderSystem::PointLightRenderSystem(vDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : device(device), renderPass(renderPass)
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(this->renderPass, 0);
    }

    PointLightRenderSystem::~PointLightRenderSystem()
    {
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void PointLightRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
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

        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout");
        }
    }

    void PointLightRenderSystem::createPipeline(VkRenderPass renderPass, int rasterMode)
    {
        assert(pipelineLayout != nullptr && "Pipeline layout must be created before pipeline");

        PipelineConfigInfo pipelineConfig{};

        vPipeline::defaultPipelineConfigInfo(pipelineConfig);
         vPipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();

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

        pipeline = std::make_unique<vPipeline>(device, pipelineConfig, "../shaders/pointLight.vert.spv", "../shaders/pointLight.frag.spv");
    }

    static float timePassed = 0.f;

    void PointLightRenderSystem::update(FrameInfo &frameInfo, UniformBufferObject &ubo)
    {
        int lightIndex = 0;

        auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.frameTime*3, {0.f, -1.f, 0.f});
        timePassed += frameInfo.frameTime*2;
        auto intensity = (glm::sin(timePassed) + 1.f) / 2.f;

        for (auto &kv : frameInfo.gameObjects)
        {
            auto &obj = kv.second;

            if (obj.pointLight == nullptr)
                continue;

            if (lightIndex >= MAX_LIGHTS)
                break;

            obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));
            obj.pointLight->intensity = intensity;
            obj.transform.scale = glm::vec3(intensity) * .1f + .05f;

            ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
            ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->intensity);

            lightIndex++;
        }

        ubo.numPointLights = lightIndex;
    }

    void PointLightRenderSystem::render(FrameInfo &frameInfo)
    {
        if (currentRasterMode != frameInfo.rasterMode)
        {
            currentRasterMode = frameInfo.rasterMode;
            createPipeline(renderPass, currentRasterMode);
        }

        std::map<float, vGameObject::id_t> sorted;
        for (auto &kv : frameInfo.gameObjects)
        {
            auto &obj = kv.second;
            if (obj.pointLight == nullptr)
                continue;

            glm::vec3 offset = frameInfo.camera.getCurrentPosition() - obj.transform.translation;
            float distanceSquared = glm::dot(offset, offset);
            sorted[distanceSquared] = obj.getId();
        }

        glm::mat4 projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

        pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

        for (auto ut = sorted.rbegin(); ut != sorted.rend(); ++ut)
        {
            auto &obj = frameInfo.gameObjects.at(ut->second);

            PointLightPushConstants push{};
            push.position = glm::vec4(obj.transform.translation, 1.f);
            push.color = glm::vec4(obj.color, obj.pointLight->intensity);
            push.radius = obj.transform.scale.x;

            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants), &push);

            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);

            // std::cout << "objects culled: " << culled << "/4096\n";
        }
    }
}