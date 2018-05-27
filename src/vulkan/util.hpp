#ifndef __WFN_ENG_VULKAN_UTIL_HPP__
#define __WFN_ENG_VULKAN_UTIL_HPP__

#include "../vulkan.hpp"

namespace wfn_eng::vulkan::util {
    ////
    // struct QueueFamilyIndices
    //
    // Utility struct that contains information on the indices of various
    // queue families (aka the IDs of VkQueues).
    struct QueueFamilyIndices {
        ////
        // int graphicsFamily
        //
        // The index of the graphics queue.
        int graphicsFamily = -1;

        ////
        // int presentationFamily
        //
        // The index of the presentation queue.
        int presentationFamily = -1;

        ////
        // int transferFamily
        //
        // The index of the tranfer queue.
        int transferFamily = -1;

        ////
        // QueueFamilyIndices(VkSurfaceKHR, VkPhysicalDevice)
        //
        // Given the reference to a VkSurfaceKHR and a VkPhysicalDevice,
        // query the relevant queue indices.
        QueueFamilyIndices(VkSurfaceKHR, VkPhysicalDevice);

        ////
        // QueueFamilyIndices(Base&, Device&)
        //
        // Given the reference to a Base and a Device, query the relevant
        // queue indices.
        QueueFamilyIndices(Base&, Device&);

        ////
        // bool sufficient
        //
        // Checks if the queue family's indices are sufficient for use in
        // the rest of the program.
        bool sufficient();
    };

    ////
    // struct SwapchainSupport
    //
    // Utility struct to encapsulate various swapchain capabilities of a
    // VkPhysicalDevice.
    struct SwapchainSupport {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        ////
        // SwapchainSupport(VkSurfaceKHR, VkPhysicalDevice)
        //
        // Queries the swapchain capabilities of a VkSurfaceKHR and a
        // VkPhysicalDevice directly.
        SwapchainSupport(VkSurfaceKHR, VkPhysicalDevice);

        ////
        // SwapchainSupport(Base&, Device&)
        //
        // Queries the swapchain capabilities of a VkPhysicalDevice via a
        // reference to the Base and Device classes (below) that encapsulate
        // the surface and physical device.
        SwapchainSupport(Base&, Device&);

        ////
        // bool sufficient
        //
        // Returns whether the queried swapchain capabilities are sufficient
        // to use.
        bool sufficient();
    };

    ////
    // struct Buffer
    //
    // Provides a wrapper around Vulkan's buffers to be more C++ like.
    struct Buffer {
        ////
        // VkDeviceSize size
        //
        // The size of the buffer.
        VkDeviceSize size;

        ////
        // VkBuffer handle
        //
        // The handle to the buffer itself.
        VkBuffer handle;

        ////
        // VkDeviceMemory memory
        //
        // The handle to the buffer's memory region.
        VkDeviceMemory memory;

        ////
        // Buffer(Device&, VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkSharingMode)
        //
        // Constructing a buffer, given the device, the size, usage flags, property
        // flags, and sharing mode.
        Buffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkSharingMode);

        ////
        // ~Buffer()
        //
        // Destroying the buffer.
        ~Buffer();

        ////
        // void map(void **)
        //
        // Maps the data in this buffer (if able) to the provided memory range.
        // Will fail if the buffer is not accessible from the CPU.
        void map(void **);

        ////
        // void unmap()
        //
        // Unmaps the above mapped data.
        void unmap();

        ////
        // void copy_to(Buffer&, VkDeviceSize, VkDeviceSize, VkDeviceSize)
        //
        // Copies the contents of this buffer to another buffer. Will halt
        // during the copy. Provides the option to specify source offset,
        // destination offset, and copy size.
        void copy_to(Buffer&, VkDeviceSize, VkDeviceSize, VkDeviceSize);

        ////
        // void copy_to(Buffer&)
        //
        // Copies the contents of this buffer to another buffer. Will halt
        // during the copy.
        void copy_to(Buffer&);

        ////
        // void copy_from(T)
        //
        // Copies the contents of the provided type into the buffer. Will fail
        // if the bufer is not accessible from the CPU.
        template <typename T>
        void copy_from(T);

        ////
        // void indirect_copy_from(T)
        //
        // Copies information from the provided type into the buffer via
        // copy_from and copy_to.
        template <typename T>
        void indirect_copy_from(T);


        // Rule of 5's.
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) = delete;
    };

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
    // struct PipelineConfig
    //
    // Provides a configuration type to define the behavior of a
    // Pipeline (including the VkRenderPasses, VkPipelineLayout, and the
    // actual VkPipeline).
    struct PipelineConfig {
        std::string vertexShaderPath;
        std::string fragmentShaderPath;

        std::vector<RenderPassConfig> renderPassConfigs;

        std::vector<VkVertexInputBindingDescription> vertexBindings;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    };

    ////
    // class Pipeline
    //
    // A wrapper around the VkPipeline that constructs a pipeline according to
    // the information provided by a PipelineConfig.
    class Pipeline {
        std::vector<VkRenderPass> _renderPasses;
        VkPipelineLayout _layout;
        VkPipeline _handle;

        void initRenderPasses(const PipelineConfig&);
        void initLayout(const PipelineConfig&);
        void initPipeline(const PipelineConfig&);

    public:
        ////
        // Pipeline(PipelineConfig)
        //
        // Constructs a Pipeline with a custom PipelineConfig.
        Pipeline(PipelineConfig);

        ////
        // Pipeline()
        //
        // Constructs a Pipeline with the default
        // PipelineConfig.
        Pipeline();

        ////
        // ~Pipeline()
        //
        // Destroys the VkRenderPass, VkPipelineLayout, and VkPipeline.
        ~Pipeline();

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

#include "util/buffer.tpp"

#endif
