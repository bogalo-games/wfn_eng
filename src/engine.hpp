#ifndef __WFN_ENG_ENGINE_HPP__
#define __WFN_ENG_ENGINE_HPP__

#include <vulkan/vulkan.h>

#include "vulkan/util.hpp"

namespace wfn_eng::engine {
    ////
    // struct Vertex
    //
    // A struct that contains relevant information for the location and color of
    // vertices.
    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;

        ////
        // VkVertexInputBindingDescription
        //
        // Generates binding information for this struct.
        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription desc = {};

            desc.binding = 0;
            desc.stride = sizeof(Vertex);
            desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return desc;
        }

        ////
        // std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions
        //
        // Generates attribute information for this struct.
        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 2> descs = {};

            descs[0].binding = 0;
            descs[0].location = 0;
            descs[0].format = VK_FORMAT_R32G32_SFLOAT;
            descs[0].offset = offsetof(Vertex, pos);

            descs[1].binding = 0;
            descs[1].location = 1;
            descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            descs[1].offset = offsetof(Vertex, color);

            return descs;
        }
    };
}

#endif
