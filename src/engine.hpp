#ifndef __WFN_ENG_ENGINE_HPP__
#define __WFN_ENG_ENGINE_HPP__

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

#include "vulkan/util.hpp"

using namespace wfn_eng::vulkan::util;

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

    enum Primitive {
        TRIANGLE,
        QUAD
    };

    struct PrimitiveRenderer {
        Pipeline *pipeline;

        Buffer *triangleBuffer;
        Buffer *triangleTransferBuffer;

        Buffer *quadBuffer;
        Buffer *quadTransferBuffer;

        Buffer *indexBuffer;
        Buffer *indexTransferBuffer;

        std::vector<VkCommandBuffer> renderCommands;
        VkCommandBuffer transferCommand;

        VkSemaphore imageAvailable;
        VkSemaphore renderFinished;

        uint32_t maxTriangles;
        uint32_t maxQuads;

        uint32_t triangleCount;
        uint32_t quadCount;

        uint32_t triangleSize();
        uint32_t quadSize();
        uint32_t indexSize();

        uint32_t triangleOffset();
        uint32_t quadOffset();
        uint32_t indexOffset();

        void clear();

        PrimitiveRenderer(size_t, size_t);

        ~PrimitiveRenderer();

        // Requests draws
        void drawTriangle(std::array<Vertex, 3>);
        void drawQuad(std::array<Vertex, 4>);
        void draw(Primitive, std::vector<Vertex>);

        // Actually draws
        void render();

        // Rule of 5's
        PrimitiveRenderer(const PrimitiveRenderer&) = delete;
        PrimitiveRenderer(PrimitiveRenderer&&) = delete;
        PrimitiveRenderer& operator=(const PrimitiveRenderer&) = delete;
        PrimitiveRenderer& operator=(PrimitiveRenderer&&) = delete;
    };
}

#endif
