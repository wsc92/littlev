#pragma once

#include "chronos_device.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Chronos {

struct PipelineConfigInfo {
    PipelineConfigInfo(const PipelineConfigInfo &) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo &) = delete;

    PipelineConfigInfo() = default;

    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class ChronosPipeline {
public:
    ChronosPipeline(
            ChronosDevice &device,
            const std::string& vertFilepath,
            const std::string& fragFilepath,
            const PipelineConfigInfo& configInfo);
    ~ChronosPipeline();

    ChronosPipeline(const ChronosPipeline&) = delete;
    ChronosPipeline& operator=(const ChronosPipeline&) = delete;

    void bind(VkCommandBuffer commandBuffer);

    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

private:
    static std::vector<char> readFile(const std::string& filepath);

    void createGraphicsPipeline(
            const std::string& vertFilepath,
            const std::string& fragFilepath,
            const PipelineConfigInfo& configInfo);
    
    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    ChronosDevice& chronosDevice;
    VkPipeline graphicsPipeline;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
};
}
