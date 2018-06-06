#include "../util.hpp"

namespace wfn_eng::vulkan::util {
    ////
    // class Pipeline
    //
    // A wrapper around the VkPipeline that constructs a pipeline according to
    // the information provided by a PipelineConfig.

    void Pipeline::initDescriptorSet(const PipelineConfig& config) {
        auto& device = Core::instance().device().logical();

        // Creating the descriptor pool
        VkDescriptorPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .poolSizeCount = static_cast<uint32_t>(config.descriptorPoolSizes.size()),
            .pPoolSizes = config.descriptorPoolSizes.data(),
            .maxSets = 1
        };

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Pipeline",
                "initDescriptorSet",
                "Failed to create descriptor pool"
            );
        }

        // Creating the descriptor set layout
        VkDescriptorSetLayoutCreateInfo layoutInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &config.descriptorSetLayoutBinding
        };

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Pipeline",
                "initDescriptorSet",
                "Failed to create descriptor set layout"
            );
        }

        // Creating the descriptor set
        VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = _descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &_descriptorSetLayout
        };

        if (vkAllocateDescriptorSets(device, &allocInfo, &_descriptorSet) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Pipeline",
                "initDescriptorSet",
                "Failed to allocate descriptor set"
            );
        }

        // TODO: Add binding 0?

        // Updating descriptor information
        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = _descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .pBufferInfo = nullptr,
            .pImageInfo = &config.descriptorImageInfo,
            .pTexelBufferView = nullptr
        };

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    void Pipeline::initRenderPasses(const PipelineConfig& config) {
        if (config.renderPassConfigs.size() == 0) {
            throw WfnError(
                "wfn_eng::engine::Pipeline",
                "initRenderPasses",
                "You must have at least one render pass"
            );
        }

        _renderPasses.resize(config.renderPassConfigs.size());
        for (int i = 0; i < config.renderPassConfigs.size(); i++) {
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

            if (vkCreateRenderPass(Core::instance().device().logical(), &renderPassInfo, nullptr, &_renderPasses[i]) != VK_SUCCESS) {
                throw WfnError(
                    "wfn_eng::engine::Pipeline",
                    "initRenderPasses",
                    "Failed to create render pass"
                );
            }
        }
    }

    void Pipeline::initLayout(const PipelineConfig& config) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        if (_hasUniform) {
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
        } else {
            pipelineLayoutInfo.setLayoutCount = 0;
            pipelineLayoutInfo.pSetLayouts = VK_NULL_HANDLE;
        }

        if (vkCreatePipelineLayout(Core::instance().device().logical(), &pipelineLayoutInfo, nullptr, &_layout) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::engine::Pipeline",
                "initLayout",
                "Failed to create layout"
            );
        }
    }

    void Pipeline::initPipeline(const PipelineConfig& config) {
        Shader vertexShader(config.vertexShaderPath);
        Shader fragmentShader(config.fragmentShaderPath);

        auto vertCreateInfo = vertexShader.shaderStage(VK_SHADER_STAGE_VERTEX_BIT);
        auto fragCreateInfo = fragmentShader.shaderStage(VK_SHADER_STAGE_FRAGMENT_BIT);

        VkPipelineShaderStageCreateInfo shaderStages[] = {
            vertCreateInfo,
            fragCreateInfo
        };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(config.vertexBindings.size());
        vertexInputInfo.pVertexBindingDescriptions = config.vertexBindings.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(config.attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = config.attributeDescriptions.data();

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
        pipelineInfo.layout = _layout;
        pipelineInfo.renderPass = _renderPasses[0];
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(Core::instance().device().logical(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_handle) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::engine::Pipeline",
                "initPipeline",
                "Failed to create pipeline"
            );
        }
    }

    ////
    // Pipeline(PipelineConfig)
    //
    // Constructs a Pipeline with a custom PipelineConfig.
    Pipeline::Pipeline(PipelineConfig config) {
        this->_hasUniform = config.hasUniform;

        if (_hasUniform)
            initDescriptorSet(config);

        initRenderPasses(config);
        initLayout(config);
        initPipeline(config);
    }

    ////
    // Pipeline()
    //
    // Constructs a Pipeline with the default
    // PipelineConfig.
    // Pipeline::Pipeline() :
    //         Pipeline(...) { } // TODO: Default config?

    ////
    // ~Pipeline()
    //
    // Destroys the VkRenderPass, VkPipelineLayout, and VkPipeline.
    Pipeline::~Pipeline() {
        auto& device = Core::instance().device().logical();

        if (_hasUniform) {
            vkDestroyDescriptorPool(device, _descriptorPool, nullptr);
            vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);
        }

        vkDestroyPipeline(device, _handle, nullptr);
        vkDestroyPipelineLayout(device, _layout, nullptr);

        for (auto& renderPass: _renderPasses)
            vkDestroyRenderPass(device, renderPass, nullptr);
    }

    ////
    // VkDescriptorPool& descriptorPool()
    //
    // Provides reference to the descriptor pool.
    VkDescriptorPool& Pipeline::descriptorPool() { return _descriptorPool; }

    ////
    // VkDescriptorSetLayout& descriptorSetLayout()
    //
    // Provides reference to the descriptor set layout.
    VkDescriptorSetLayout& Pipeline::descriptorSetLayout() { return _descriptorSetLayout; }

    ////
    // VkDescriptorSet& descriptorSet()
    //
    // Provides reference to the descriptor set.
    VkDescriptorSet& Pipeline::descriptorSet() { return _descriptorSet; }

    ////
    // std::vector<VkRenderPass>& renderPasses()
    //
    // Provides reference to the render pass.
    std::vector<VkRenderPass>& Pipeline::renderPasses() { return _renderPasses; }

    ////
    // VkPipelineLayout& layout()
    //
    // Provides reference to the pipeline layout.
    VkPipelineLayout& Pipeline::layout() { return _layout; }

    ////
    // VkPipeline& handle()
    //
    // Provides reference to the pipeline itself.
    VkPipeline& Pipeline::handle() { return _handle; }
}
