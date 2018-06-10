#include "vertex.hpp"

namespace wfn_eng::engine {
    ////
    // struct Vertex
    //
    // A struct that contains relevant information for the location and color of
    // vertices.

    ////
    // VkVertexInputBindingDescription
    //
    // Generates binding information for this struct.
    VkVertexInputBindingDescription Vertex::getBindingDescription() {
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
    std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> descs = {};

        descs[0].binding = 0;
        descs[0].location = 0;
        descs[0].format = VK_FORMAT_R32G32_SFLOAT;
        descs[0].offset = offsetof(Vertex, pos);

        descs[1].binding = 0;
        descs[1].location = 1;
        descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        descs[1].offset = offsetof(Vertex, color);

        descs[2].binding = 0;
        descs[2].location = 2;
        descs[2].format = VK_FORMAT_R32G32_SFLOAT;
        descs[2].offset = offsetof(Vertex, texPos);

        return descs;
    }
}
