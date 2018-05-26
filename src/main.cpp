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

#include "vulkan/util.hpp"
#include "vulkan.hpp"
#include "sdl.hpp"

const int WIDTH  = 640;
const int HEIGHT = 480;

#define DEBUG

using namespace wfn_eng;
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

////
// readFile
//
// Reads an entire file into a std::vector of characters.
static std::vector<char> readFile(std::string path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file");

    size_t len = file.tellg();
    file.seekg(0);

    std::vector<char> buf(len);
    file.read(buf.data(), len);

    file.close();

    return buf;
}

////
// makeShader
//
// Constructs a VkShaderModule.
static VkShaderModule makeShader(Core& core, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule module;
    if (vkCreateShaderModule(core.device().logical(), &createInfo, nullptr, &module))
        throw std::runtime_error("Failed to create shader module");

    return module;
}

////
// VkPipelineShaderStageCreateInfo shaderCreateInfo(VkShaderModule, SfkShaderStageFlagBits)
//
// Because vertex and fragment shaders have really similar stage creation, I've
// just gone and put it in a function.
static VkPipelineShaderStageCreateInfo shaderCreateInfo(VkShaderModule module, VkShaderStageFlagBits stage) {
    VkPipelineShaderStageCreateInfo createInfo = {};

    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage = stage;
    createInfo.module = module;
    createInfo.pName = "main";

    return createInfo;
}

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

class HelloTriangleApplication {
private:
    Window *window;

    // Graphics pipeline
    VkRenderPass renderPass;
    VkPipelineLayout graphicsPipelineLayout;
    VkPipeline graphicsPipeline;

    // Command buffers
    util::Buffer *graphicsBuffer;
    util::Buffer *indexBuffer;
    util::Buffer *transferBuffer;
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

    void initRenderPass() {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = Core::instance().swapchain().format();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(Core::instance().device().logical(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("Could not create render pass");
    }

    void initGraphicsPipeline() {
        initRenderPass();

        VkShaderModule vertModule = makeShader(Core::instance(), readFile(vertPath));
        VkShaderModule fragModule = makeShader(Core::instance(), readFile(fragPath));

        auto vertCreateInfo = shaderCreateInfo(vertModule, VK_SHADER_STAGE_VERTEX_BIT);
        auto fragCreateInfo = shaderCreateInfo(fragModule, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkPipelineShaderStageCreateInfo shaderStages[] = {
            vertCreateInfo,
            fragCreateInfo
        };

        auto bindingDesc = Vertex::getBindingDescription();
        auto attributeDescs = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescs.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)Core::instance().swapchain().extent().width;
        viewport.height = (float)Core::instance().swapchain().extent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = Core::instance().swapchain().extent();

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if(vkCreatePipelineLayout(Core::instance().device().logical(), &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics pipeline layout");

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr; // Optional
        pipelineInfo.layout = graphicsPipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(Core::instance().device().logical(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics pipeline");
    }

    void createIndexBuffer() {
        indexBuffer = new util::Buffer(
            Core::instance().device(),
            indices.size() * sizeof(uint16_t),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SHARING_MODE_CONCURRENT
        );

        util::Buffer stagingBuffer(
            Core::instance().device(),
            indices.size() * sizeof(uint16_t),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            VK_SHARING_MODE_CONCURRENT
        );

        stagingBuffer.copy_from(Core::instance().device(), indices.data());
        stagingBuffer.copy_to(Core::instance().device(), Core::instance().commandPools().transfer(), *indexBuffer);
    }

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

        graphicsBuffer = new util::Buffer(
            Core::instance().device(),
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SHARING_MODE_CONCURRENT
        );

        indexBuffer = new util::Buffer(
            Core::instance().device(),
            indices.size() * sizeof(uint16_t),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SHARING_MODE_CONCURRENT
        );

        indexBuffer->indirect_copy_from(Core::instance().device(), Core::instance().commandPools().transfer(), indices.data());

        transferBuffer = new util::Buffer(
            Core::instance().device(),
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            VK_SHARING_MODE_CONCURRENT
        );

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
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = Core::instance().swapchain().frameBuffers()[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = Core::instance().swapchain().extent();

            VkClearValue clearColor = { 0.3f, 0.3f, 0.3f, 1.0f };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(graphicsCommands[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(graphicsCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkBuffer graphicsBuffers[] = { graphicsBuffer->handle };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(graphicsCommands[i], 0, 1, graphicsBuffers, offsets);

            vkCmdBindIndexBuffer(graphicsCommands[i], indexBuffer->handle, 0, VK_INDEX_TYPE_UINT16);

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
        vkCmdCopyBuffer(transferCommands[0], transferBuffer->handle, graphicsBuffer->handle, 1, &copyRegion);

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

        initGraphicsPipeline();
        initCommandBuffers();
        initSemaphores();
    }

    ////
    // Destroying everything
    void cleanupGraphicsPipeline() {
        vkDestroyPipeline(Core::instance().device().logical(), graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(Core::instance().device().logical(), graphicsPipelineLayout, nullptr);
        vkDestroyRenderPass(Core::instance().device().logical(), renderPass, nullptr);
    }

    void cleanupCommandBuffers() {
        delete graphicsBuffer;
        delete indexBuffer;
        delete transferBuffer;
    }

    void cleanupSemaphores() {
        vkDestroySemaphore(Core::instance().device().logical(), imageAvailable, nullptr);
        vkDestroySemaphore(Core::instance().device().logical(), renderFinished, nullptr);
    }

    void cleanup() {
        cleanupGraphicsPipeline();
        cleanupCommandBuffers();
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

        transferBuffer->copy_from(Core::instance().device(), mv_verts.data());

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
