#pragma once

#include "chronos_device.hpp"
#include "chronos_game_object.hpp"
#include "chronos_pipeline.hpp"
#include "chronos_window.hpp"
#include "chronos_renderer.hpp"

//std
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

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
        void loadGameObjects();
        void createPipelineLayout();
        void createPipeline();
        void renderGameObjects(VkCommandBuffer commandBuffer);

    private:
        ChronosWindow chronosWindow{WIDTH, HEIGHT, "HELLO VULKAN!"};
        ChronosDevice chronosDevice{chronosWindow};
        ChronosRenderer chronosRenderer{chronosWindow, chronosDevice};

        std::unique_ptr<ChronosSwapChain> chronosSwapChain;
        std::unique_ptr<ChronosPipeline> chronosPipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<ChronosGameObject> gameObjects;

    };
}
