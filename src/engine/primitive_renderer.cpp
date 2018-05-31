#include "../engine.hpp"

using namespace wfn_eng::vulkan;

namespace wfn_eng::engine {
    uint32_t PrimitiveRenderer::triangleSize() { return sizeof(Vertex) * 3; }
    uint32_t PrimitiveRenderer::quadSize() { return sizeof(Vertex) * 4; }

    uint32_t PrimitiveRenderer::triangleOffset() { return triangleCount * triangleSize(); }
    uint32_t PrimitiveRenderer::quadOffset() { return quadCount * quadSize(); }
    uint32_t PrimitiveRenderer::indexOffset() { return quadCount * sizeof(uint16_t); }

    void PrimitiveRenderer::clear() {
        triangleTransferBuffer->clear();
        quadTransferBuffer->clear();
        indexTransferBuffer->clear();

        triangleCount = 0;
        quadCount = 0;
    }

    PrimitiveRenderer::PrimitiveRenderer(size_t maxTriangles, size_t maxQuads) {
        auto& core = Core::instance();

        // Constructing the graphics pipeline
        auto ad = Vertex::getAttributeDescriptions();
        PipelineConfig pipelineConfig {
            .vertexShaderPath = "src/shaders/vert.spv",
            .fragmentShaderPath = "src/shaders/frag.spv",

            .renderPassConfigs = std::vector<RenderPassConfig> { RenderPassConfig { } },

            .vertexBindings = std::vector<VkVertexInputBindingDescription> {
                Vertex::getBindingDescription()
            },

            .attributeDescriptions = std::vector<VkVertexInputAttributeDescription>(ad.begin(), ad.end())
        };

        pipeline = new Pipeline(pipelineConfig);

        // Constructing the buffers
        triangleBuffer = new Buffer(
            maxTriangles * triangleSize(),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SHARING_MODE_CONCURRENT
        );

        quadBuffer = new Buffer(
            maxQuads * quadSize(),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SHARING_MODE_CONCURRENT
        );

        indexBuffer = new Buffer(
            maxQuads * 6 * sizeof(uint16_t),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SHARING_MODE_CONCURRENT
        );

        // Constructing the transfer buffers
        triangleTransferBuffer = new Buffer(
            maxTriangles * triangleSize(),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            VK_SHARING_MODE_CONCURRENT
        );

        quadTransferBuffer = new Buffer(
            maxQuads * quadSize(),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            VK_SHARING_MODE_CONCURRENT
        );

        indexTransferBuffer = new Buffer(
            maxQuads * 6 * sizeof(uint16_t),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            VK_SHARING_MODE_CONCURRENT
        );

        // Constructing render commands
        renderCommands.resize(core.swapchain().frameBuffers().size());

        VkCommandBufferAllocateInfo renderAllocateInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = core.commandPools().graphics(),
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = (uint32_t)renderCommands.size()
        };

        if (vkAllocateCommandBuffers(core.device().logical(), &renderAllocateInfo, renderCommands.data()) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::engine::PrimitiveRenderer",
                "PrimitiveRenderer",
                "Could not allocate render commands."
            );
        }

        for (int i = 0; i < renderCommands.size(); i++) {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

            if (vkBeginCommandBuffer(renderCommands[i], &beginInfo) != VK_SUCCESS) {
                throw WfnError(
                    "wfn_eng::engine::PrimitiveRenderer",
                    "PrimitiveRenderer",
                    "Failed to begin recording render command"
                );
            }

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = pipeline->renderPasses()[0];
            renderPassInfo.framebuffer = core.swapchain().frameBuffers()[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = core.swapchain().extent();

            VkClearValue clearColor = { 0.3f, 0.3f, 0.3f, 1.0f };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(renderCommands[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(renderCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle());

            // Rendering triangles
            VkBuffer triangleBufferRef[] = { triangleBuffer->handle };
            VkDeviceSize triangleBufferOffset[] = { 0 };
            vkCmdBindVertexBuffers(renderCommands[i], 0, 1, triangleBufferRef, triangleBufferOffset);

            vkCmdDraw(
                renderCommands[i],
                3 * maxTriangles,
                1,
                0,
                0
            );

            // Rendering quads
            VkBuffer quadBufferRef[] = { quadBuffer->handle };
            VkDeviceSize quadBufferOffset[] = { 0 };
            vkCmdBindVertexBuffers(renderCommands[i], 0, 1, quadBufferRef, quadBufferOffset);
            vkCmdBindIndexBuffer(renderCommands[i], indexBuffer->handle, 0, VK_INDEX_TYPE_UINT16);

            vkCmdDrawIndexed(
                renderCommands[i],
                6 * maxQuads,
                1,
                0,
                0,
                0
            );

            vkCmdEndRenderPass(renderCommands[i]);

            if (vkEndCommandBuffer(renderCommands[i]) != VK_SUCCESS) {
                throw WfnError(
                    "wfn_eng::engine::PrimitiveRenderer",
                    "PrimitiveRenderer",
                    "Failed to record render command"
                );
            }
        }

        // Constructing transfer commands
        VkCommandBufferAllocateInfo transferAllocateInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = core.commandPools().transfer(),
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        if (vkAllocateCommandBuffers(core.device().logical(), &transferAllocateInfo, &transferCommand) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::engine::PrimitiveRenderer",
                "PrimitiveRenderer",
                "Could not allocate transfer command"
            );
        };


        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

        if (vkBeginCommandBuffer(transferCommand, &beginInfo) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::engine::PrimitiveRenderer",
                "PrimitiveRenderer",
                "Failed to begin recording transfer command"
            );
        }

        VkBufferCopy copyTriangles = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = maxTriangles * 3 * sizeof(Vertex)
        };

        VkBufferCopy copyQuads = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = maxQuads * 4 * sizeof(Vertex)
        };

        VkBufferCopy copyIndices = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = maxQuads * 6 * sizeof(uint16_t)
        };

        vkCmdCopyBuffer(transferCommand, triangleTransferBuffer->handle, triangleBuffer->handle, 1, &copyTriangles);
        vkCmdCopyBuffer(transferCommand, quadTransferBuffer->handle, quadBuffer->handle, 1, &copyQuads);
        vkCmdCopyBuffer(transferCommand, indexTransferBuffer->handle, indexBuffer->handle, 1, &copyIndices);

        if (vkEndCommandBuffer(transferCommand) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::engine::PrimitiveRenderer",
                "PrimitiveRenderer",
                "Failed to record transfer command"
            );
        }

        // Constructing semaphores
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(core.device().logical(), &semaphoreInfo, nullptr, &imageAvailable) != VK_SUCCESS ||
            vkCreateSemaphore(core.device().logical(), &semaphoreInfo, nullptr, &renderFinished) != VK_SUCCESS) {

            throw WfnError(
                "wfn_eng::engine::PrimitiveRenderer",
                "PrimitiveRenderer",
                "Failed to create semaphores"
            );
        }

        // Initializing book-keeping
        this->maxTriangles = maxTriangles;
        this->maxQuads = maxQuads;

        triangleCount = 0;
        quadCount = 0;
    }

    PrimitiveRenderer::~PrimitiveRenderer() {
        // NOTE: graphics and transfer buffers get deleted when their command
        //       pool (constructed via Core) gets deleted.

        vkDestroySemaphore(Core::instance().device().logical(), imageAvailable, nullptr);
        vkDestroySemaphore(Core::instance().device().logical(), renderFinished, nullptr);

        delete triangleBuffer;
        delete triangleTransferBuffer;
        delete quadBuffer;
        delete quadTransferBuffer;
        delete indexBuffer;
        delete indexTransferBuffer;
        delete pipeline;
    }

    void PrimitiveRenderer::drawTriangle(std::array<Vertex, 3> vert) {
        if (triangleCount >= maxTriangles) {
            throw WfnError(
                "wfn_eng::engine::PrimitiveRenderer",
                "drawTriangle",
                "Cannot exceed the maximum triangle count"
            );
        }

        void *data;
        triangleTransferBuffer->map(&data);
        memcpy((Vertex *)data + 3 * triangleCount, vert.data(), sizeof(Vertex) * 3);
        triangleTransferBuffer->unmap();

        triangleCount += 1;
    }

    void PrimitiveRenderer::drawQuad(std::array<Vertex, 4> vert) {
        if (quadCount >= maxQuads) {
            throw WfnError(
                "wfn_eng::engine::PrimitiveRenderer",
                "drawQuad",
                "Cannot exceed the maximum quad count"
            );
        }

        void *data;
        quadTransferBuffer->map(&data);
        memcpy((Vertex *)data + 4 * quadCount, vert.data(), sizeof(Vertex) * 4);
        quadTransferBuffer->unmap();

        std::array<uint16_t, 6> indices { 0, 1, 2, 2, 3, 0 };
        for (auto& index: indices)
            index += 4 * quadCount;

        indexTransferBuffer->map(&data);
        memcpy((uint16_t *)data + 6 * quadCount, indices.data(), sizeof(uint16_t) * 6);

        quadCount += 1;
    }

    void PrimitiveRenderer::draw(Primitive kind, std::vector<Vertex> vertices) {
        if (kind == Primitive::TRIANGLE) {
            if (vertices.size() % 3 != 0) {
                throw WfnError(
                    "wfn_eng::engine::PrimitiveRenderer",
                    "draw",
                    "TRIANGLE types must have a multiple of 3 indices"
                );
            }

            for (int i = 0; (i * 3) < vertices.size(); i++) {
                drawTriangle(
                    std::array<Vertex, 3> {
                        vertices[i * 3],
                        vertices[i * 3 + 1],
                        vertices[i * 3 + 2]
                    }
                );
            }
        }

        if (kind == Primitive::QUAD) {
            if (vertices.size() % 4 != 0) {
                throw WfnError(
                    "wfn_eng::engine::PrimitiveRenderer",
                    "draw",
                    "QUAD types must have a multiple of 4 indices"
                );
            }

            for (int i = 0; (i * 4) < vertices.size(); i++) {
                drawQuad(
                    std::array<Vertex, 4> {
                        vertices[i * 4],
                        vertices[i * 4 + 1],
                        vertices[i * 4 + 2],
                        vertices[i * 4 + 3]
                    }
                );
            }
        }
    }

    void PrimitiveRenderer::render() {
        auto& core = Core::instance();

        // Copying over the data to the graphics buffers
        VkSubmitInfo transferSubmitInfo = {};
        transferSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        transferSubmitInfo.commandBufferCount = 1;
        transferSubmitInfo.pCommandBuffers = &transferCommand;

        if (vkQueueSubmit(core.device().transferQueue(), 1, &transferSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::engine::PrimitiveRenderer",
                "render",
                "Failed to submit transfer command"
            );
        }

        // Performing the render
        uint32_t imageIndex;
        vkAcquireNextImageKHR(
            Core::instance().device().logical(),
            Core::instance().swapchain().get(),
            std::numeric_limits<uint64_t>::max(),
            imageAvailable,
            VK_NULL_HANDLE,
            &imageIndex
        );

        VkSubmitInfo renderSubmitInfo = {};
        renderSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailable };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        renderSubmitInfo.waitSemaphoreCount = 1;
        renderSubmitInfo.pWaitSemaphores = waitSemaphores;
        renderSubmitInfo.pWaitDstStageMask = waitStages;

        renderSubmitInfo.commandBufferCount = 1;
        renderSubmitInfo.pCommandBuffers = &renderCommands[imageIndex];

        VkSemaphore signalSemaphores[] = { renderFinished };
        renderSubmitInfo.signalSemaphoreCount = 1;
        renderSubmitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(Core::instance().device().graphicsQueue(), 1, &renderSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::engine::PrimitiveRenderer",
                "render",
                "Failed to submit render command"
            );
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapchains[] = { Core::instance().swapchain().get() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        vkQueuePresentKHR(Core::instance().device().presentationQueue(), &presentInfo);

        // Clearing the staging buffers
        clear();
    }
}
