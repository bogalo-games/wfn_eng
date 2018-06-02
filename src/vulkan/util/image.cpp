#include "../util.hpp"

namespace wfn_eng::vulkan::util {
    ////
    // struct Image
    //
    // A wrapper around Vulkan's VkImage to handle creation and destruction.

    ////
    // Image(uint32_t, uint32_t, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags)
    //
    // Constructs a new Image from the provided information.
    Image::Image(uint32_t width, uint32_t height, VkFormat format,
                 VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags memoryProps, VkSharingMode sharing) {
        auto& device = Core::instance().device().logical();

        VkImageCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .extent.width = width,
            .extent.height = height,
            .extent.depth = 1,
            .mipLevels = 1,
            .arrayLayers = 1,
            .format = format,
            .tiling = tiling,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .usage = usage,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .sharingMode = sharing
        };

        if (vkCreateImage(device, &createInfo, nullptr, &handle) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Image",
                "Image",
                "Could not create image handle"
            );
        }

        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(device, handle, &requirements);

        VkMemoryAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = requirements.size,
            .memoryTypeIndex = findMemoryType(
                requirements.memoryTypeBits,
                memoryProps
            )
        };

        if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Image",
                "Image",
                "Failed to allocate image memory"
            );
        }
    }

    ////
    // ~Image()
    //
    // Destroys the image and its contents.
    Image::~Image() {
        auto& device = Core::instance().device().logical();

        vkFreeMemory(device, memory, nullptr);
        vkDestroyImage(device, handle, nullptr);
    }

    ////
    // void transitionLayout(VkImageLayout)
    //
    // Moves from the current layout to another layout.
    void Image::transitionLayout(VkImageLayout layout) {
        // TODO: this
    }
}
