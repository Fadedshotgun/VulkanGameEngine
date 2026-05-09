#pragma once

#include "vDevice.hpp"
#include "vModel.hpp"
#include "vSwapChain.hpp"
#include "vWindow.hpp"

#include <array>
#include <memory>
#include <vector>

namespace v
{
    class vRenderer
    {
      public:
        vRenderer(vWindow &window, vDevice &device);
        ~vRenderer();

        vRenderer(const vRenderer &) = delete;
        vRenderer &operator=(const vRenderer &) = delete;

        VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
        float getAspectRatio() const { return swapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const
        {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const
        {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChain(VkCommandBuffer commandBuffer);
        void endSwapChain(VkCommandBuffer commandBuffer);
        void editRenderArea(VkCommandBuffer commandBuffer, glm::vec4 offsetFromBorders);

      private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        vWindow &window;
        vDevice &device;
        std::unique_ptr<vSwapChain> swapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};
    };
}