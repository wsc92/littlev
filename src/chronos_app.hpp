#pragma once

#include "chronos_device.hpp"
#include "chronos_pipeline.hpp"
#include "chronos_swap_chain.hpp"
#include "chronos_window.hpp"
#include "chronos_model.hpp"

//std
#include <memory>
#include <vector>

namespace Chronos {
    class ChronosApp {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

    public:
        ChronosApp();
        ~ChronosApp();

        ChronosApp(const ChronosApp &) = delete;
        ChronosApp &operator=(const ChronosApp &) = delete;

        void run();
    private:
        void loadModels();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void freeCommandBuffers();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);

    private:
        ChronosWindow chronosWindow{WIDTH, HEIGHT, "HELLO VULKAN!"};
        ChronosDevice chronosDevice{chronosWindow};
        std::unique_ptr<ChronosSwapChain> chronosSwapChain;
        std::unique_ptr<ChronosPipeline> chronosPipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::unique_ptr<ChronosModel> chronosModel;

    };
}
