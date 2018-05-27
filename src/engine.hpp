#ifndef __WFN_ENG_ENGINE_HPP__
#define __WFN_ENG_ENGINE_HPP__

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace wfn_eng::engine {
    ////
    // class Shader
    //
    // Provides a wrapper around the VkShaderModule class to handle construction
    // and module stage info.
    class Shader {
        VkShaderModule _module;

        void init(const std::vector<char>&);

    public:
        ////
        // Shader(const std::vector<char>&)
        //
        // Constructs a shader from the provided code.
        Shader(const std::vector<char>&);

        ////
        // Shader(std::string)
        //
        // Constructs a shader from code located in a file.
        Shader(std::string);

        ////
        // ~Shader()
        //
        // Destroys the VkShaderModule
        ~Shader();

        ////
        // VkShaderModule& module()
        //
        // Returns the shader module of this shader.
        VkShaderModule& module();

        ////
        // VkPipelineShaderStageCreateInfo shaderStage(VkShaderStageFlagBits)
        //
        // Creates a VkPipelineShaderStageCreateInfo for this shader.
        VkPipelineShaderStageCreateInfo shaderStage(VkShaderStageFlagBits);


        // Rule of 5's
        Shader(const Shader&) = delete;
        Shader(Shader&&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader& operator=(Shader&&) = delete;
    };

    ////
    // struct RenderPassConfig
    //
    // Provides a configuration type to define the behavior of a VkRenderPass.
    struct RenderPassConfig {

    };

    ////
    // struct GraphicsPipelineConfig
    //
    // Provides a configuration type to define the behavior of a
    // GraphicsPipeline (including the VkRenderPasses, VkPipelineLayout, and the
    // actual VkPipeline).
    struct GraphicsPipelineConfig {
        std::string vertexShaderPath;
        std::string fragmentShaderPath;

        std::vector<RenderPassConfig> renderPassConfigs;

        std::vector<VkVertexInputBindingDescription> vertexBindings;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    };

    ////
    // class GraphicsPipeline
    //
    // A wrapper around the VkPipeline that constructs a pipeline according to
    // the information provided by a GraphicsPipelineConfig.
    class GraphicsPipeline {
        std::vector<VkRenderPass> _renderPasses;
        VkPipelineLayout _layout;
        VkPipeline _pipeline;

        void initRenderPasses(const GraphicsPipelineConfig&);
        void initLayout(const GraphicsPipelineConfig&);
        void initPipeline(const GraphicsPipelineConfig&);

    public:
        ////
        // GraphicsPipeline(GraphicsPipelineConfig)
        //
        // Constructs a GraphicsPipeline with a custom GraphicsPipelineConfig.
        GraphicsPipeline(GraphicsPipelineConfig);

        ////
        // GraphicsPipeline()
        //
        // Constructs a GraphicsPipeline with the default
        // GraphicsPipelineConfig.
        GraphicsPipeline();

        ////
        // ~GraphicsPipeline()
        //
        // Destroys the VkRenderPass, VkPipelineLayout, and VkPipeline.
        ~GraphicsPipeline();

        ////
        // std::vector<VkRenderPass>& renderPasses()
        //
        // Provides reference to the render passes.
        std::vector<VkRenderPass>& renderPasses();

        ////
        // VkPipelineLayout& layout()
        //
        // Provides reference to the pipeline layout.
        VkPipelineLayout& layout();

        ////
        // VkPipeline& handle()
        //
        // Provides reference to the pipeline itself.
        VkPipeline& handle();
    };
}

#endif
