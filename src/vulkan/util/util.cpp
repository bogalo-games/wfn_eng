#include "../util.hpp"

namespace wfn_eng::vulkan::util {
    ////
    // uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags)
    //
    // Chooses a type of memory given a type filter and
    // VkMemoryPropertyFlags.
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(Core::instance().device().physical(), &memProps);

        for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
            if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
                return i;

        throw std::runtime_error("Failed to find a suitable memory type");
    }
}
