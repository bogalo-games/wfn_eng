#ifndef __WFN_ENG_ENGINE_VERTEX_HPP__
#define __WFN_ENG_ENGINE_VERTEX_HPP__

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

namespace wfn_eng::engine {
    ////
    // struct Vertex
    //
    // A struct that contains relevant information for the location and color of
    // vertices.
    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;
        glm::vec2 texPos;

        ////
        // VkVertexInputBindingDescription
        //
        // Generates binding information for this struct.
        static VkVertexInputBindingDescription getBindingDescription();

        ////
        // std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions
        //
        // Generates attribute information for this struct.
        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    };
}

#endif
