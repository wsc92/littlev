#pragma once

#include "chronos_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std
#include <vector>

namespace Chronos {
    class ChronosModel {
    public:
        struct Vertex
        {
            glm::vec2 position;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        ChronosModel(ChronosDevice &device, const std::vector<Vertex> &vertices);
        ~ChronosModel();

        ChronosModel(const ChronosModel &) = delete;
        ChronosModel &operator=(const ChronosModel &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);

    private:
        ChronosDevice& chronosDevice;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };
}
