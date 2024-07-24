#include "chronos_app.hpp"
#include "chronos_pipeline.hpp"
#include "chronos_swap_chain.hpp"
#include <vulkan/vulkan_core.h>

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std
#include <array>
#include <cassert>
#include <stdexcept>
#include <vector>

namespace Chronos {

    // TODO: TEMP restructure this!!!
    struct SimplePushConstantData 
    {
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };


    ChronosApp::ChronosApp()
    {
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    ChronosApp::~ChronosApp()
    {
        vkDestroyPipelineLayout(chronosDevice.device(), pipelineLayout, nullptr);
    }

    void ChronosApp::run() {
        while (!chronosWindow.shouldClose()) {
            glfwPollEvents();
            drawFrame();
            
        }
        vkDeviceWaitIdle(chronosDevice.device());
    }

    void ChronosApp::loadModels()
    {
        std::vector<ChronosModel::Vertex> vertices 
        {
            {{ 0.0f,-0.5f}, {1.0f, 0.0f, 0.0f}},
            {{ 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        };

        chronosModel = std::make_unique<ChronosModel>(chronosDevice, vertices);
    }

    void ChronosApp::createPipelineLayout()
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT || VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // FIX: Should set to one
        pipelineLayoutInfo.pSetLayouts = nullptr; // FIX: should reference the descriptorSetLayouts
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(chronosDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to creaet pipeline layout");
        }
    }

    void ChronosApp::createPipeline()
    {
        assert(chronosSwapChain != nullptr && "Cannot create pipeline before swap chain!");
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

        PipelineConfigInfo pipelineConfig{}; 
        ChronosPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = chronosSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        chronosPipeline = std::make_unique<ChronosPipeline>(
                chronosDevice,
                "/home/cogent/dev/vengine/src/shaders/simple_shader.vert.spv",
                "/home/cogent/dev/vengine/src/shaders/simple_shader.frag.spv",
                pipelineConfig);
    }

    void ChronosApp::recreateSwapChain()
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
        createPipeline();
    }

    void ChronosApp::createCommandBuffers() 
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

    void ChronosApp::freeCommandBuffers()
    {
        vkFreeCommandBuffers(
                chronosDevice.device(),
                chronosDevice.getCommandPool(),
                static_cast<uint32_t>(commandBuffers.size()),
                commandBuffers.data());
        commandBuffers.clear();
    }
    

    void ChronosApp::recordCommandBuffer(int imageIndex)
    {

        static int frame = 0;
        frame = (frame + 1) % 1000;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = chronosSwapChain->getRenderPass();
        renderPassInfo.framebuffer = chronosSwapChain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0,0};
        renderPassInfo.renderArea.extent = chronosSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        
        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(chronosSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(chronosSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0,0}, chronosSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffers[imageIndex], 0,1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0,1, &scissor);

        chronosPipeline->bind(commandBuffers[imageIndex]);
        chronosModel->bind(commandBuffers[imageIndex]);

        for (int j = 0; j < 4; j++) 
        {
            SimplePushConstantData push{};
            push.offset = {-0.5f + frame * 0.002f, -0.4 + j * 0.25f};
            push.color = {0.0f, 0.0f, 0.2f * j};

            vkCmdPushConstants(
                    commandBuffers[imageIndex],
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(SimplePushConstantData),
                    &push);

            chronosModel->draw(commandBuffers[imageIndex]);
        }

        vkCmdEndRenderPass(commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void ChronosApp::drawFrame() 
    {
        uint32_t imageIndex;
        auto result = chronosSwapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        recordCommandBuffer(imageIndex);
        result = chronosSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || chronosWindow.wasWindowResized())
        {
            chronosWindow.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }
}
