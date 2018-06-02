#include "../util.hpp"

using namespace wfn_eng::vulkan;
using namespace wfn_eng;

static VkCommandBuffer beginOTCommandBuffer() {
    auto& transferPool = Core::instance().commandPools().transfer();
    auto& device = Core::instance().device();

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = transferPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(device.logical(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw WfnError(
            "wfn_eng::util::Buffer",
            "copy_to",
            "Failed to allocate copy command buffer"
        );
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw WfnError(
            "wfn_eng::util::Buffer",
            "copy_to",
            "Failed to begin command buffer"
        );
    }

    return commandBuffer;
}

static void endOTCommandBuffer(VkCommandBuffer commandBuffer) {
    auto& device = Core::instance().device();

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw WfnError(
            "wfn_eng::util::Buffer",
            "copy_to",
            "Failed to end command buffer"
        );
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (vkQueueSubmit(device.transferQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw WfnError(
            "wfn_eng::util::Buffer",
            "copy_to",
            "Failed to submit command buffer"
        );
    }

    if (vkQueueWaitIdle(device.transferQueue()) != VK_SUCCESS) {
        throw WfnError(
            "wfn_eng::util::Buffer",
            "copy_to",
            "Failed to wait for queue idle"
        );
    }
}

namespace wfn_eng::vulkan::util {
    ////
    // struct Buffer
    //
    // Provides a wrapper around Vulkan's buffers to be more C++ like.

    ////
    // Buffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkSharingMode)
    //
    // Constructing a buffer, given the device, the size, usage flags, property
    // flags, and sharing mode.
    Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkSharingMode sharingMode) {
        auto& device = Core::instance().device();

        this->size = size;

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = sharingMode;

        if (vkCreateBuffer(device.logical(), &bufferInfo, nullptr, &handle) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::util::Buffer",
                "Buffer",
                "Failed to allocate buffer"
            );
        }

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(device.logical(), handle, &memReq);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = findMemoryType(
            memReq.memoryTypeBits,
            props
        );

        if (vkAllocateMemory(device.logical(), &allocInfo, nullptr, &memory) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::util::Buffer",
                "Buffer",
                "Failed to allocate memory"
            );
        }

        vkBindBufferMemory(device.logical(), handle, memory, 0);
    }

    ////
    // ~Buffer()
    //
    //
    Buffer::~Buffer() {
        auto& device = Core::instance().device().logical();
        vkDestroyBuffer(device, handle, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }

    ////
    // void map(void **)
    //
    // Maps the data in this buffer (if able) to the provided memory range.
    void Buffer::map(void **data) {
        if (vkMapMemory(Core::instance().device().logical(), memory, 0, size, 0, data) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::util::Buffer",
                "map",
                "Failed to map memory"
            );
        }
    }

    ////
    // void unmap()
    //
    // Unmaps the above mapped data.
    void Buffer::unmap() {
        vkUnmapMemory(Core::instance().device().logical(), memory);
    }

    ////
    // void clear()
    //
    // Zeros out the entire buffer.
    void Buffer::clear() {
        void *data;
        map(&data);
        memset(data, 0, size);
        unmap();
    }

    ////
    // void copy_to(VkCommandPool, Buffer&, VkDeviceSize, VkDeviceSize, VkDeviceSize)
    //
    // Copies the contents of this buffer to another buffer. Will halt
    // during the copy. Provides the option to specify source offset,
    // destination offset, and copy size.
    void Buffer::copy_to(Buffer& buffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginOTCommandBuffer();

        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = srcOffset;
        copyRegion.dstOffset = dstOffset;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, handle, buffer.handle, 1, &copyRegion);

        endOTCommandBuffer(commandBuffer);
    }

    ////
    // void copy_to(VkCommandPool, Buffer&)
    //
    // Copies the contents of this buffer to another buffer. Will halt
    // during the copy.
    void Buffer::copy_to(Buffer& buffer) {
        copy_to(
            buffer,
            0,
            0,
            size
        );
    }

    ////
    // void copy_to(Image&, VkDeviceSize, VkDeviceSize, VkDeviceSize)
    //
    // Copies the contents of this buffer to an image. Will halt during the
    // copy. Provides the option to specify source offset, destination
    // offset, and copy size.
    void Buffer::copy_to(Image& image, VkDeviceSize srcOffset, VkOffset3D dstOffset, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginOTCommandBuffer();

        VkBufferImageCopy region = {
            .bufferOffset = srcOffset,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,

            .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .imageSubresource.mipLevel = 0,
            .imageSubresource.baseArrayLayer = 0,
            .imageSubresource.layerCount = 1,
            .imageOffset = { 0, 0, 0 },
            .imageExtent = {
                image.width,
                image.height,
                1
            }
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            handle,
            image.handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        endOTCommandBuffer(commandBuffer);
    }

    ////
    // void copy_to(Image&)
    //
    // Copies the contents of this buffer to n image. Will halt during the
    // copy.
    void Buffer::copy_to(Image& image) {
        copy_to(
            image,
            0,
            VkOffset3D { 0, 0, 0 },
            size
        );
    }
}
