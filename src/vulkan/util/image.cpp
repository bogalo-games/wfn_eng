#include "../util.hpp"

namespace wfn_eng::vulkan::util {
    ////
    // struct Image
    //
    // A wrapper around Vulkan's VkImage to handle creation and destruction.

    ////
    // Image(uint32_t, uint32_t, int, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags)
    //
    // Constructs a new Image from the provided information.
    Image::Image(uint32_t width, uint32_t height, int channels,
                 VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags memoryProps, VkSharingMode sharing) {
        auto& device = Core::instance().device().logical();

        this->width = width;
        this->height = height;
        this->layout = VK_IMAGE_LAYOUT_UNDEFINED;
        this->channels = channels;
        if (channels == 4)
            format = VK_FORMAT_R8G8B8A8_UNORM;
        else {
            throw WfnError(
                "wfn_eng::vulkan::util::Image",
                "Image",
                "Unsupported channel count."
            );
        }

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

        vkBindImageMemory(device, handle, memory, 0);
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
        auto& transferPool = Core::instance().commandPools().transfer();
        auto& device = Core::instance().device();

        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandPool = transferPool,
            .commandBufferCount = 1
        };

        VkCommandBuffer buf;
        if (vkAllocateCommandBuffers(device.logical(), &allocInfo, &buf) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Image",
                "transitionLayout",
                "Failed to create transition buffer"
            );
        }

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        if (vkBeginCommandBuffer(buf, &beginInfo) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Image",
                "transitionLayout",
                "Failed to begin transfer buffer"
            );
        }

        VkImageMemoryBarrier barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout = this->layout,
            .newLayout = layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = handle,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };

        VkPipelineStageFlags srcStage, dstStage;
        if (this->layout == VK_IMAGE_LAYOUT_UNDEFINED && layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (this->layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (this->layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (this->layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw WfnError(
                "wfn_eng::vulkan::util::Image",
                "transitionLayout",
                "Unsupported transition"
            );
        }

        vkCmdPipelineBarrier(
            buf,
            srcStage, dstStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        if (vkEndCommandBuffer(buf) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Image",
                "transitionLayout",
                "Failed to record command buffer"
            );
        }

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &buf
        };

        if (vkQueueSubmit(device.transferQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Image",
                "transitionLayout",
                "Failed to submit to queue"
            );
        }

        if (vkQueueWaitIdle(device.transferQueue()) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Image",
                "transitionLayout",
                "Failed to wait for queue idle"
            );
        }

        this->layout = layout;
    }
}
