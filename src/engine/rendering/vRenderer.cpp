#include "vRenderer.hpp"

namespace v
{
    vRenderer::vRenderer(vWindow &window, vDevice &device) : window(window), device(device)
    {
        recreateSwapChain();
        createCommandBuffers();
    }

    vRenderer::~vRenderer()
    {
        freeCommandBuffers();
    }

    void vRenderer::recreateSwapChain()
    {
        auto extent = window.getExtent();

        while (extent.width == 0 || extent.height == 0)
        {
            extent = window.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device.device());

        if (swapChain == nullptr)
        {
            swapChain = std::make_unique<vSwapChain>(device, extent);
        }
        else
        {
            std::shared_ptr<vSwapChain> oldSwapChain = std::move(swapChain);
            swapChain = std::make_unique<vSwapChain>(device, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*swapChain.get()))
            {
                throw std::runtime_error("Swap chain image(or depth) format has changed");
            }
        }
    }

    void vRenderer::createCommandBuffers()
    {
        commandBuffers.resize(vSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffers");
        }
    }

    void vRenderer::freeCommandBuffers()
    {
        vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    VkCommandBuffer vRenderer::beginFrame()
    {
        assert(!isFrameStarted && "Cannot call beginFrame while already in progress");

        VkResult result = swapChain->acquireNextImage(&currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        isFrameStarted = true;

        VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin command buffer");
        }

        return commandBuffer;
    }

    void vRenderer::endFrame()
    {
        assert(isFrameStarted && "Cannot call endFrame while frame not in progress");

        VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to end command buffer");
        }

        VkResult result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasFrameBufferResized())
        {
            window.resetResizedFlag();
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swap chain image");
        }

        isFrameStarted = false;

        currentFrameIndex = (currentFrameIndex + 1) % vSwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void vRenderer::editRenderArea(VkCommandBuffer commandBuffer, glm::vec4 offsetFromBorders)
    {
        VkViewport viewport{};
        viewport.x = offsetFromBorders.x;
        viewport.y = offsetFromBorders.y;
        viewport.width = offsetFromBorders.z;
        viewport.height = offsetFromBorders.w;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {static_cast<int32_t>(offsetFromBorders.x), static_cast<int32_t>(offsetFromBorders.y)};
        scissor.extent = {static_cast<uint32_t>(offsetFromBorders.z), static_cast<uint32_t>(offsetFromBorders.w)};
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void vRenderer::beginSwapChain(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Cannot call beginSwapChain if frame not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cannot begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapChain->getRenderPass();
        renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.003F, 0.003F, 0.003F, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);   // - 50;
        viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height); // - 50;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, swapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void vRenderer::endSwapChain(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Cannot call endSwapChain if frame not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cannot end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);
    }
}