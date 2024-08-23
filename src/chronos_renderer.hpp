#pragma once

#include "chronos_device.hpp"
#include "chronos_swap_chain.hpp"
#include "chronos_window.hpp"

//std
#include <memory>
#include <vector>
#include <cassert>

namespace Chronos {
    class ChronosRenderer {
    public:

    public:
        ChronosRenderer(ChronosWindow &window, ChronosDevice &device);
        ~ChronosRenderer();

        ChronosRenderer(const ChronosRenderer &) = delete;
        ChronosRenderer &operator=(const ChronosRenderer &) = delete;

        VkRenderPass getSwapChainRenderPass() const { return chronosSwapChain->getRenderPass(); }
        bool isFrameInProgress() const { return isFrameStarted;}

        VkCommandBuffer getCurrentCommandBuffer() const 
        {
            assert(isFrameStarted && "Cannot get command buffer when frame is not in progress");
            return commandBuffers[currentImageIndex];
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
        

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

    private:
        ChronosWindow& chronosWindow;
        ChronosDevice& chronosDevice;
        std::unique_ptr<ChronosSwapChain> chronosSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        bool isFrameStarted;
    };
}
