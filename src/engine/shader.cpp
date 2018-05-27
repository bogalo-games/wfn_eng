#include "../engine.hpp"

#include "../vulkan.hpp"
#include <fstream>

using namespace wfn_eng::vulkan;

namespace wfn_eng::engine {
    ////
    // class Shader
    //
    // Provides a wrapper around the VkShaderModule class to handle construction
    // and module stage info.

    void Shader::init(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        if (vkCreateShaderModule(Core::instance().device().logical(), &createInfo, nullptr, &_module)) {
            throw WfnError(
                "wfn_eng::engine::Shader",
                "init",
                "Failed to create shader module"
            );
        }
    }

    ////
    // Shader(const std::vector<char>&)
    //
    // Constructs a shader from the provided code.
    Shader::Shader(const std::vector<char>& code) { init(code); }

    ////
    // Shader(std::string)
    //
    // Constructs a shader from code located in a file.
    Shader::Shader(std::string path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("Failed to open file");

        size_t len = file.tellg();
        file.seekg(0);

        std::vector<char> buf(len);
        file.read(buf.data(), len);

        file.close();

        init(buf);
    }

    ////
    // ~Shader()
    //
    // Destroys the VkShaderModule
    Shader::~Shader() {
        vkDestroyShaderModule(
            Core::instance().device().logical(),
            _module,
            nullptr
        );
    }

    ////
    // VkShaderModule& module()
    //
    // Returns the shader module of this shader.
    VkShaderModule& Shader::module() { return _module; }

    ////
    // VkPipelineShaderStageCreateInfo shaderStage(VkShaderStageFlagBits)
    //
    // Creates a VkPipelineShaderStageCreateInfo for this shader.
    VkPipelineShaderStageCreateInfo Shader::shaderStage(VkShaderStageFlagBits stage) {
        VkPipelineShaderStageCreateInfo createInfo = {};

        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage = stage;
        createInfo.module = _module;
        createInfo.pName = "main";

        return createInfo;
    }
}
