#include <vulkan/vulkan.h>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <glm/glm.hpp>

#include <functional>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <array>
#include <cmath>
#include <set>

#include "engine.hpp"
#include "sdl.hpp"
#include "vulkan/util.hpp"
#include "vulkan.hpp"

const int WIDTH  = 640;
const int HEIGHT = 480;

#define DEBUG

using namespace wfn_eng;
using namespace wfn_eng::engine;
using namespace wfn_eng::sdl;
using namespace wfn_eng::vulkan;
using namespace wfn_eng::vulkan::util;

////
// vertPath
// fragPath
//
// Paths to both the vertex and fragment shaders.
const static std::string vertPath = "src/shaders/vert.spv";
const static std::string fragPath = "src/shaders/frag.spv";

class HelloTriangleApplication {
private:
    Window *window;

    PrimitiveRenderer *renderer;

    // Command buffers
    std::vector<VkCommandBuffer> graphicsCommands;
    std::vector<VkCommandBuffer> transferCommands;

    // Semaphore
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;

    // Vertex definition
    const std::vector<Vertex> vertices = {
        { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
        { {  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
        { {  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
        { { -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f } }
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    ////
    // Initializing Vulkan components
    void initCommandBuffers() {
        //
        // Creating the command pool
        //
        graphicsCommands.resize(Core::instance().swapchain().frameBuffers().size());
        transferCommands.resize(1); // TODO: Change this later?

        //
        // Creating the vertex buffers
        //
        // VkPhysicalDevice physical, VkDevice device,
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        renderer->indexBuffer->indirect_copy_from(indices.data());

        //
        // Creating the draw commands
        //
        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = Core::instance().commandPools().graphics();
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = (uint32_t)graphicsCommands.size();

        if (vkAllocateCommandBuffers(Core::instance().device().logical(), &allocateInfo, graphicsCommands.data()))
            throw std::runtime_error("Failed to allocate command buffers");

        for (size_t i = 0; i < graphicsCommands.size(); i++) {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

            if (vkBeginCommandBuffer(graphicsCommands[i], &beginInfo) != VK_SUCCESS)
                throw std::runtime_error("Failed to begin recording command buffer");

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderer->pipeline->renderPasses()[0];
            renderPassInfo.framebuffer = Core::instance().swapchain().frameBuffers()[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = Core::instance().swapchain().extent();

            VkClearValue clearColor = { 0.3f, 0.3f, 0.3f, 1.0f };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(graphicsCommands[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(graphicsCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline->handle());

            VkBuffer graphicsBuffers[] = { renderer->quadBuffer->handle };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(graphicsCommands[i], 0, 1, graphicsBuffers, offsets);

            vkCmdBindIndexBuffer(graphicsCommands[i], renderer->indexBuffer->handle, 0, VK_INDEX_TYPE_UINT16);

            vkCmdDrawIndexed(
                graphicsCommands[i],
                static_cast<uint32_t>(indices.size()),
                1,
                0,
                0,
                0
            );

            vkCmdEndRenderPass(graphicsCommands[i]);

            if (vkEndCommandBuffer(graphicsCommands[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to record command buffer");
        }

        //
        // Creating the transfer command
        //
        VkCommandBufferAllocateInfo transferInfo = {};
        transferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        transferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        transferInfo.commandPool = Core::instance().commandPools().transfer();
        transferInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(Core::instance().device().logical(), &transferInfo, transferCommands.data());

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        vkBeginCommandBuffer(transferCommands[0], &beginInfo);

        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = bufferSize;
        vkCmdCopyBuffer(transferCommands[0], renderer->quadTransferBuffer->handle, renderer->quadBuffer->handle, 1, &copyRegion);

        if (vkEndCommandBuffer(transferCommands[0]) != VK_SUCCESS)
            throw std::runtime_error("Failed to record transfer buffer");
    }

    void initSemaphores() {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(Core::instance().device().logical(), &semaphoreInfo, nullptr, &imageAvailable) != VK_SUCCESS ||
            vkCreateSemaphore(Core::instance().device().logical(), &semaphoreInfo, nullptr, &renderFinished) != VK_SUCCESS) {

            throw std::runtime_error("Failed to create semaphores");
        }
    }

    void init() {
        WindowConfig cfg {
            .vulkanPath = "vulkan/macOS/lib/libvulkan.1.dylib",
            .windowName = "Testing Vulkan",
            .width = WIDTH,
            .height = HEIGHT,
            .flags = 0
        };

        window = new Window(cfg);
        Core::initialize(*window);

        renderer = new PrimitiveRenderer(1, 1);

        initCommandBuffers();
        initSemaphores();
    }

    ////
    // Destroying everything
    void cleanupSemaphores() {
        vkDestroySemaphore(Core::instance().device().logical(), imageAvailable, nullptr);
        vkDestroySemaphore(Core::instance().device().logical(), renderFinished, nullptr);
    }

    void cleanup() {
        delete renderer;
        cleanupSemaphores();

        Core::destroy();
        delete window;
    }

    ////
    // Game Logic
    void updatePosition(const glm::vec2& pos, uint32_t ticks) {
        float t = ticks / 1000.0f;

        std::vector<Vertex> mv_verts(vertices);
        for (int i = 0; i < mv_verts.size(); i++) {
            float dx = cos(t * (1 + i)) / 8;
            float dy = cos(t * 2 * (1 + i)) / 8;

            mv_verts[i].pos = mv_verts[i].pos + pos + glm::vec2(dx , dy);
        }

        renderer->quadTransferBuffer->copy_from(mv_verts.data());

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = transferCommands.data();

        vkQueueSubmit(Core::instance().device().transferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    }

    void drawFrame() {
        uint32_t imageIndex;
        vkAcquireNextImageKHR(
            Core::instance().device().logical(),
            Core::instance().swapchain().get(),
            std::numeric_limits<uint64_t>::max(),
            imageAvailable,
            VK_NULL_HANDLE,
            &imageIndex
        );

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailable };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &graphicsCommands[imageIndex];

        VkSemaphore signalSemaphores[] = { renderFinished };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(Core::instance().device().graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit queue");

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
    }

    void mainLoop() {
        bool quit = false;
        SDL_Event event;

        bool up = false, down = false, left = false, right = false;
        glm::vec2 pos(0, 0);

        uint32_t last = 0;
        while (true) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT)
                    quit = true;
                else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_w) up = true;
                    if (event.key.keysym.sym == SDLK_s) down = true;
                    if (event.key.keysym.sym == SDLK_a) left = true;
                    if (event.key.keysym.sym == SDLK_d) right = true;
                } else if (event.type == SDL_KEYUP) {
                    if (event.key.keysym.sym == SDLK_w) up = false;
                    if (event.key.keysym.sym == SDLK_s) down = false;
                    if (event.key.keysym.sym == SDLK_a) left = false;
                    if (event.key.keysym.sym == SDLK_d) right = false;
                }
            }

            uint32_t curr = SDL_GetTicks();

            int dx = 0;
            if (left)  dx -= 1;
            if (right) dx += 1;

            int dy = 0;
            if (up)   dy -= 1;
            if (down) dy += 1;

            pos += glm::vec2(dx, dy) * ((curr - last) / 1000.0f);

            if (quit == true)
                break;

            updatePosition(pos, curr);
            drawFrame();
            last = curr;

            SDL_Delay(16);
        }

        vkDeviceWaitIdle(Core::instance().device().logical());
    }

public:
    void run() {
        init();
        mainLoop();
        cleanup();
    }
};

int main() {
    HelloTriangleApplication app;
    try {
        app.run();
    } catch (const std::runtime_error& e) {
        std::cerr << "App error: " << e.what() << std::endl;
        return 1;
    } catch (const WfnError& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
