#include "chronos_renderer.hpp"

//std
#include <array>
#include <cassert>
#include <stdexcept>
#include <vector>

namespace Chronos {

    ChronosRenderer::ChronosRenderer(ChronosWindow &window, ChronosDevice &device) : chronosWindow{window}, chronosDevice{device}
    {
        recreateSwapChain();
        createCommandBuffers();
    }

    ChronosRenderer::~ChronosRenderer()
    {
        freeCommandBuffers();
    }

    void ChronosRenderer::recreateSwapChain()
    {
        auto extent = chronosWindow.getExtent();
        while (extent.width == 0 || extent.height == 0) 
        {
            extent = chronosWindow.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(chronosDevice.device());

        if (chronosSwapChain == nullptr) 
        {
            chronosSwapChain = std::make_unique<ChronosSwapChain>(chronosDevice, extent);
        } else {
            chronosSwapChain = std::make_unique<ChronosSwapChain>(chronosDevice, extent, std::move(chronosSwapChain));
            if (chronosSwapChain->imageCount() != commandBuffers.size())
            {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }
        // come back here
    }

    void ChronosRenderer::createCommandBuffers() 
    {
        commandBuffers.resize(chronosSwapChain->imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = chronosDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(chronosDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void ChronosRenderer::freeCommandBuffers()
    {
        vkFreeCommandBuffers(
                chronosDevice.device(),
                chronosDevice.getCommandPool(),
                static_cast<uint32_t>(commandBuffers.size()),
                commandBuffers.data());
        commandBuffers.clear();
    }

    VkCommandBuffer ChronosRenderer::beginFrame()
    {
        assert(!isFrameStarted && "Can't call beginFrame while already in progress");
        auto result = chronosSwapChain->acquireNextImage(&currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        return commandBuffer;
    }
    void ChronosRenderer::endFrame()
    {
        assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
        auto result = chronosSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || chronosWindow.wasWindowResized())
        {
            chronosWindow.resetWindowResizedFlag();
            recreateSwapChain();
        } else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }
        isFrameStarted = false;
    }
    void ChronosRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = chronosSwapChain->getRenderPass();
        renderPassInfo.framebuffer = chronosSwapChain->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = {0,0};
        renderPassInfo.renderArea.extent = chronosSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(chronosSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(chronosSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0,0}, chronosSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0,1, &viewport);
        vkCmdSetScissor(commandBuffer, 0,1, &scissor);
    }
    void ChronosRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);

    }

}
