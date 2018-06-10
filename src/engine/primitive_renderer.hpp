#ifndef __WFN_ENG_ENGINE_PRIMITIVE_RENDERER_HPP__
#define __WFN_ENG_ENGINE_PRIMITIVE_RENDERER_HPP__

// TODO Vertex decl

#include "../vulkan/util.hpp"
#include "vertex.hpp"

using namespace wfn_eng::vulkan::util;

namespace wfn_eng::engine {
    enum Primitive {
        TRIANGLE,
        QUAD
    };

    class PrimitiveRenderer {
        Texture *texture;

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

        VkFence transferFinished;

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

    public:
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
