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
        vkMapMemory(device.logical(), memory, 0, size, 0, data);
    }

    ////
    // void unmap(Device&)
    //
    // Unmaps the above mapped data.
    void Buffer::unmap(Device& device) {
        vkUnmapMemory(device.logical(), memory);
    }
}
