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
    // struct Image
    //
    // A wrapper around Vulkan's VkImage to handle creation and destruction.
    struct Image {
        ////
        // uint32_t width
        //
        // The width, in pixels, of the image
        uint32_t width;

        ////
        // uint32_t height
        //
        // The height, in pixels, of the image.
        uint32_t height;

        ////
        // VkImage handle
        //
        // The handle to the Vulkan implementation of an image.
        VkImage handle;

        ////
        // VkDeviceMemory memory
        //
        // The device memory mapped to the image.
        VkDeviceMemory memory;

        ////
        // VkImageLayout layout
        //
        // The layout of the image. Don't modify this without using
        // transitionLayout please.
        VkImageLayout layout;

        ////
        // int channels
        //
        // The number of channels in the image.
        int channels;

        ////
        // VkFormat format
        //
        // The format of the image, as decided per the number of channels.
        VkFormat format;

        ////
        // Image(uint32_t, uint32_t, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags)
        //
        // Constructs a new Image from the provided information.
        Image(uint32_t, uint32_t, int, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkSharingMode);

        ////
        // ~Image()
        //
        // Destroys the image and its contents.
        ~Image();

        ////
        // void transitionLayout(VkImageLayout)
        //
        // Moves from the current layout to another layout.
        void transitionLayout(VkImageLayout);


        // Rule of 5's.
        Image(const Image&) = delete;
        Image(Image&&) = delete;
        Image& operator=(const Image&) = delete;
        Image& operator=(Image&&) = delete;
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
        // void clear()
        //
        // Zeros out the entire buffer.
        void clear();

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
        // void copy_to(Image&, VkDeviceSize, VkDeviceSize, VkDeviceSize)
        //
        // Copies the contents of this buffer to an image. Will halt during the
        // copy. Provides the option to specify source offset, destination
        // offset, and copy size.
        void copy_to(Image&, VkDeviceSize, VkOffset3D, VkDeviceSize);

        ////
        // void copy_to(Image&)
        //
        // Copies the contents of this buffer to n image. Will halt during the
        // copy.
        void copy_to(Image&);

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

        bool hasUniform;

        std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
        VkDescriptorImageInfo descriptorImageInfo;

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
        VkDescriptorPool _descriptorPool;
        VkDescriptorSetLayout _descriptorSetLayout;
        VkDescriptorSet _descriptorSet;

        std::vector<VkRenderPass> _renderPasses;
        VkPipelineLayout _layout;
        VkPipeline _handle;

        bool _hasUniform;

        void initDescriptorSet(const PipelineConfig&);
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
        // VkDescriptorPool& descriptorPool()
        //
        // Provides reference to the descriptor pool.
        VkDescriptorPool& descriptorPool();

        ////
        // VkDescriptorSetLayout& descriptorSetLayout()
        //
        // Provides reference to the descriptor set layout.
        VkDescriptorSetLayout& descriptorSetLayout();

        ////
        // VkDescriptorSet& descriptorSet()
        //
        // Provides reference to the descriptor set.
        VkDescriptorSet& descriptorSet();

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

    ////
    // class Texture
    //
    // A class built on top of the Buffer and Image structs to provide texture
    // sampling to fragment shaders.
    class Texture {
        Image *_image;
        VkImageView _imageView;
        VkSampler _sampler;

        void createImage(std::string);
        void createImageView();
        void createSampler(VkFilter);

    public:
        ////
        // Texture(std::string, VkFilter)
        //
        // Builds a texture from a path on disk, and provides the option of
        // VkFilter for both the mag and min filter.
        Texture(std::string, VkFilter);

        ////
        // Texture(std::string)
        //
        // Builds a texture from a path on disk.
        Texture(std::string);

        ////
        // ~Texture()
        //
        //
        ~Texture();

        ////
        // Image& image
        //
        // Provides a reference to the internal image.
        Image& image();

        ////
        // VkImageView& imageView()
        //
        // Provides a reference to the VkImageView.
        VkImageView& imageView();

        ////
        // VkSampler& sampler();
        //
        // Provides a reference to the VkSampler.
        VkSampler& sampler();


        // Rule of 5's.
        Texture(const Texture&) = delete;
        Texture(Texture&&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture& operator=(Texture&&) = delete;
    };

    ////
    // uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags)
    //
    // Chooses a type of memory given a type filter and
    // VkMemoryPropertyFlags.
    uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags);
}

#include "util/buffer.tpp"

#endif
