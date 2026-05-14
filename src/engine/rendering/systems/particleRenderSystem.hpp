#pragma once

#include "hash.hpp"
#include "vCamera.hpp"
#include "vDevice.hpp"
#include "vFrameInfo.hpp"
#include "vPipeline.hpp"

#include <array>
#include <memory>
#include <vector>

namespace v
{
    class ParticleRenderSystem
    {
      public:
        ParticleRenderSystem(vDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~ParticleRenderSystem();

        ParticleRenderSystem(const ParticleRenderSystem &) = delete;
        ParticleRenderSystem &operator=(const ParticleRenderSystem &) = delete;

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