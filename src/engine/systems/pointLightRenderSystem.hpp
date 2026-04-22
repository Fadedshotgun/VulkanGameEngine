#pragma once

#include "hash.hpp"
#include "vCamera.hpp"
#include "vDevice.hpp"
#include "vFrameInfo.hpp"
#include "vGameObject.hpp"
#include "vPipeline.hpp"

#include <array>
#include <memory>
#include <vector>

namespace v
{
    class PointLightRenderSystem
    {
      public:
        PointLightRenderSystem(vDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightRenderSystem();

        PointLightRenderSystem(const PointLightRenderSystem &) = delete;
        PointLightRenderSystem &operator=(const PointLightRenderSystem &) = delete;

        void update(FrameInfo &frameInfo, UniformBufferObject &ubo);
        void render(FrameInfo &frameInfo);

        int triangleCount{0};

      private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass, int rasterMode);

        vDevice &device;
        VkRenderPass renderPass;

        std::unique_ptr<vPipeline> pipeline;
        VkPipelineLayout pipelineLayout;
        int currentRasterMode{0};
    };
}