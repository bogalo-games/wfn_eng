#include "../util.hpp"

namespace wfn_eng::vulkan::util {
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
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount > 0) {
            formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                device,
                surface,
                &formatCount,
                formats.data()
            );
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount > 0) {
            presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                surface,
                &presentModeCount,
                presentModes.data()
            );
        }
    }

    ////
    // SwapchainSupport(Base&, Device&)
    //
    // Queries the swapchain capabilities of a VkPhysicalDevice via a
    // reference to the Base and Device classes (below) that encapsulate
    // the surface and physical device.
    SwapchainSupport::SwapchainSupport(Base& base, Device& device) :
            SwapchainSupport(base.surface(), device.physical()) { }

    ////
    // bool sufficient
    //
    // Returns whether the queried swapchain capabilities are sufficient
    // to use.
    bool SwapchainSupport::sufficient() {
        return formats.size() > 0 && presentModes.size() > 0;
    }
}
