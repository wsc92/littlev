#!/bin/bash

${VULKAN_SDK}/bin/glslc src/shaders/simple_shader.vert -o src/shaders/simple_shader.vert.spv
${VULKAN_SDK}/bin/glslc src/shaders/simple_shader.frag -o src/shaders/simple_shader.frag.spv
