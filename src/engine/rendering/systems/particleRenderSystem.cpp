#include "particleRenderSystem.hpp"

#include "particleEmitterSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#define RENDER_DISTANCE 100

namespace v
{
    struct ParticleInstance
    {
        glm::vec3 position{};
        float size{};
        glm::vec4 color{};
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
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

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

        pipelineConfig.bindingDescriptions = {{0, sizeof(ParticleInstance), VK_VERTEX_INPUT_RATE_INSTANCE}};
        pipelineConfig.attributeDescriptions = {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ParticleInstance, position)},
            {1, 0, VK_FORMAT_R32_SFLOAT, offsetof(ParticleInstance, size)},
            {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(ParticleInstance, color)},
        };
        vPipeline::enableAlphaBlending(pipelineConfig);

        // might wanna figure this out for transparency later
        // pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
        // pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;

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

    void ParticleRenderSystem::resizeInstanceBuffer(ecs::ParticleEmitterComponent &emitter)
    {
        if (emitter.particles.size() <= emitter.instanceData.capacity)
            return;

        vkDeviceWaitIdle(device.device());

        std::cout << "Resizing particle instance buffer capacity" << std::endl;

        if (emitter.instanceData.buffer != VK_NULL_HANDLE)
        {
            ecs::destroyParticleEmitterInstanceData(emitter, device);
        }

        size_t newCapacity = static_cast<size_t>(emitter.emissionRate * emitter.lifetime) + 1;
        if (emitter.particles.size() > newCapacity)
        {
            newCapacity = emitter.particles.size();
        }

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(ParticleInstance) * newCapacity;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vkCreateBuffer(device.device(), &bufferInfo, nullptr, &emitter.instanceData.buffer);

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device.device(), emitter.instanceData.buffer, &memoryRequirements);

        VkMemoryAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.allocationSize = memoryRequirements.size;
        allocateInfo.memoryTypeIndex = device.findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkAllocateMemory(device.device(), &allocateInfo, nullptr, &emitter.instanceData.memory);
        vkBindBufferMemory(device.device(), emitter.instanceData.buffer, emitter.instanceData.memory, 0);
        vkMapMemory(device.device(), emitter.instanceData.memory, 0, bufferInfo.size, 0, &emitter.instanceData.mappedPointer);

        emitter.instanceData.capacity = newCapacity;
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
                // if (emitter.particles.empty())
                //     return;

                // resizeInstanceBuffer(emitter);

                // auto *particleInstance = static_cast<ParticleInstance *>(emitter.instanceData.mappedPointer);
                // for (size_t i = 0; i < emitter.particles.size(); i++)
                // {
                //     auto &particle = emitter.particles[i];
                //     particleInstance[i].position = particle.position;
                //     particleInstance[i].size = particle.size / 100.f;
                //     particleInstance[i].color = glm::vec4(particle.color, 1);
                // }

                // VkBuffer vertexBuffers[] = {emitter.instanceData.buffer};
                // VkDeviceSize offsets[] = {0};
                // vkCmdBindVertexBuffers(frameInfo.commandBuffer, 0, 1, vertexBuffers, offsets);

                // vkCmdDraw(frameInfo.commandBuffer, 6, static_cast<uint32_t>(emitter.particles.size()), 0, 0);
                // for (auto particle : emitter.particles)
                // {
                //     ParticlePushConstantData push{};
                //     push.position = glm::vec4(particle.position, 1.f);
                //     push.color = glm::vec4(particle.color, 1);
                //     push.size = particle.size / 100;

                //     vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ParticlePushConstantData), &push);

                //     vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
                // }
            });
    }
}