#pragma once

#include "hash.hpp"
#include "vCamera.hpp"
#include "vDevice.hpp"
#include "vGameObject.hpp"
#include "vPipeline.hpp"
#include "vFrameInfo.hpp"

#include <array>
#include <memory>
#include <vector>

namespace v
{
    class DefaultRenderSystem
    {
      public:
        DefaultRenderSystem(vDevice &device,  VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout, VkDescriptorSet defaultTextureSet);
        ~DefaultRenderSystem();

        DefaultRenderSystem(const DefaultRenderSystem &) = delete;
        DefaultRenderSystem &operator=(const DefaultRenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo);

        int triangleCount{0};

      private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
        void createPipeline(VkRenderPass renderPass, int rasterMode);

        vDevice &device;
        VkRenderPass renderPass;

        std::unique_ptr<vPipeline> pipeline;
        VkPipelineLayout pipelineLayout;
        int currentRasterMode{0};
        VkDescriptorSet defaultTextureSet = VK_NULL_HANDLE;
    };
}