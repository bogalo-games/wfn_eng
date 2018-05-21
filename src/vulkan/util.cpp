#include "../vulkan.hpp"

namespace wfn_eng::vulkan::util {
    ////
    // struct QueueFamilyIndices
    //
    // Utility struct that contains information on the indices of various
    // queue families (aka the IDs of VkQueues).

    ////
    // QueueFamilyIndices(VkSurfaceKHR, VkPhysicalDevice)
    //
    // Given the reference to a VkSurfaceKHR and a VkPhysicalDevice,
    // query the relevant queue indices.
    QueueFamilyIndices::QueueFamilyIndices(VkSurfaceKHR surface, VkPhysicalDevice device) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &queueFamilyCount,
            nullptr
        );

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &queueFamilyCount,
            queueFamilies.data()
        );

        int i = 0;
        for (int i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphicsFamily = i;

            VkBool32 presentationSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);
            if (queueFamilies[i].queueCount > 0 && presentationSupport)
                presentationFamily = i;

            if (sufficient())
                break;
        }
    }

    ////
    // QueueFamilyIndices(Base&, Device&)
    //
    // Given the reference to a Base and a Device, query the relevant
    // queue indices.
    //
    // TODO: Impl once we have Base and Device implemented
    QueueFamilyIndices::QueueFamilyIndices(Base& base, Device& device) { }

    ////
    // bool sufficient
    //
    // Checks if the queue family's indices are sufficient for use in
    // the rest of the program.
    bool QueueFamilyIndices::sufficient() {
        return graphicsFamily >= 0 && presentationFamily >= 0;
    }

    ////
    // struct SwapchainSupport
    //
    // Utility struct to encapsulate various swapchain capabilities of a
    // VkPhysicalDevice.

    ////
    // SwapchainSupport(VkSurfaceKHR, VkPhysicalDevice)
    //
    // Queries the swapchain capabilities of a VkSurfaceKHR and a
    // VkPhysicalDevice directly.
    SwapchainSupport::SwapchainSupport(VkSurfaceKHR surface, VkPhysicalDevice device) {

    }

    ////
    // SwapchainSupport(Base&, Device&)
    //
    // Queries the swapchain capabilities of a VkPhysicalDevice via a
    // reference to the Base and Device classes (below) that encapsulate
    // the surface and physical device.
    //
    // TODO: Impl once we have Base and Device implemented
    SwapchainSupport::SwapchainSupport(Base& base, Device& device) { }

    ////
    // bool sufficient
    //
    // Returns whether the queried swapchain capabilities are sufficient
    // to use.
    bool SwapchainSupport::sufficient() {
        return formats.size() > 0 && presentModes.size() > 0;
    }
}
