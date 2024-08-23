#include "chronos_app.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

//std
#include <cassert>
#include <stdexcept>
#include <vector>

namespace Chronos {

    // TODO: TEMP restructure this!!!
    struct SimplePushConstantData 
    {
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };


    ChronosApp::ChronosApp()
    {
        loadGameObjects();
        createPipelineLayout();
        createPipeline();
    }

    ChronosApp::~ChronosApp()
    {
        vkDestroyPipelineLayout(chronosDevice.device(), pipelineLayout, nullptr);
    }

    void ChronosApp::run() {
        while (!chronosWindow.shouldClose()) {
            glfwPollEvents();
            
            if (auto commandBuffer = chronosRenderer.beginFrame()) {
                chronosRenderer.beginSwapChainRenderPass(commandBuffer);
                renderGameObjects(commandBuffer);
                chronosRenderer.endSwapChainRenderPass(commandBuffer);
                chronosRenderer.endFrame();
            }
        }
        vkDeviceWaitIdle(chronosDevice.device());
    }

    void ChronosApp::loadGameObjects()
    {
        std::vector<ChronosModel::Vertex> vertices 
        {
            {{ 0.0f,-0.5f}, {1.0f, 0.0f, 0.0f}},
            {{ 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        };

        auto chronosModel = std::make_shared<ChronosModel>(chronosDevice, vertices);

        auto triangle = ChronosGameObject::createGameObject();
        triangle.model = chronosModel;
        triangle.color = {.1f, .8f, .1f};
        triangle.transform2d.translation.x = .2f;
        triangle.transform2d.scale = {2.f, .5f};
        triangle.transform2d.rotation = .25f * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
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
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

        PipelineConfigInfo pipelineConfig{}; 
        ChronosPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = chronosRenderer.getSwapChainRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        chronosPipeline = std::make_unique<ChronosPipeline>(
                chronosDevice,
                "/home/cogent/dev/vengine/src/shaders/simple_shader.vert.spv",
                "/home/cogent/dev/vengine/src/shaders/simple_shader.frag.spv",
                pipelineConfig);
    }

    void ChronosApp::renderGameObjects(VkCommandBuffer commandBuffer)
    {
        chronosPipeline->bind(commandBuffer);

        for (auto& obj: gameObjects)
        {
            SimplePushConstantData push{};
            push.offset = obj.transform2d.translation;
            push.color = obj.color;
            push.transform = obj.transform2d.mat2();

            vkCmdPushConstants(
                    commandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(SimplePushConstantData),
                    &push);
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
            
        }
    }
}
