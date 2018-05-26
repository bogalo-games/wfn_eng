#include "../util.hpp"

////
// uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags)
//
// Chooses the type of memory to use for our buffer.
static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
            return i;

    throw std::runtime_error("Failed to find a suitable memory type");
}

namespace wfn_eng::vulkan::util {
    ////
    // struct Buffer
    //
    // Provides a wrapper around Vulkan's buffers to be more C++ like.

    ////
    // Buffer(Device&, VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkSharingMode)
    //
    // Constructing a buffer, given the device, the size, usage flags, property
    // flags, and sharing mode.
    Buffer::Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkSharingMode sharingMode) :
            _device(device) {
        this->size = size;

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = sharingMode;

        if (vkCreateBuffer(device.logical(), &bufferInfo, nullptr, &handle) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Buffer",
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
            device.physical(),
            memReq.memoryTypeBits,
            props
        );

        if (vkAllocateMemory(device.logical(), &allocInfo, nullptr, &memory) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Buffer",
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
        vkDestroyBuffer(_device.logical(), handle, nullptr);
        vkFreeMemory(_device.logical(), memory, nullptr);
    }

    ////
    // void map(Device&, void **)
    //
    // Maps the data in this buffer (if able) to the provided memory range.
    void Buffer::map(Device& device, void **data) {
        if (vkMapMemory(device.logical(), memory, 0, size, 0, data) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Buffer",
                "map",
                "Failed to map memory"
            );
        }
    }

    ////
    // void unmap(Device&)
    //
    // Unmaps the above mapped data.
    void Buffer::unmap(Device& device) {
        vkUnmapMemory(device.logical(), memory);
    }

    ////
    // void copy_to(Device&, VkCommandPool, Buffer&, VkDeviceSize, VkDeviceSize, VkDeviceSize)
    //
    // Copies the contents of this buffer to another buffer. Will halt
    // during the copy. Provides the option to specify source offset,
    // destination offset, and copy size.
    void Buffer::copy_to(Device& device, VkCommandPool transferPool, Buffer& buffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size) {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = transferPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        if (vkAllocateCommandBuffers(device.logical(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
            // TODO: Send error
        }

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            // TODO: Send error
        }

        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = srcOffset;
        copyRegion.dstOffset = dstOffset;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, handle, buffer.handle, 1, &copyRegion);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            // TODO: Send error
        }

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        if (vkQueueSubmit(device.transferQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            // TODO: Send error
        }

        if (vkQueueWaitIdle(device.transferQueue()) != VK_SUCCESS) {
            // TODO: Send error
        }
    }

    ////
    // void copy_to(Device&, VkCommandPool, Buffer&)
    //
    // Copies the contents of this buffer to another buffer. Will halt
    // during the copy.
    void Buffer::copy_to(Device& device, VkCommandPool transferPool, Buffer& buffer) {
        copy_to(
            device,
            transferPool,
            buffer,
            0,
            0,
            size
        );
    }
}
