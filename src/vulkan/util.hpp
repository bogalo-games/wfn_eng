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
}

#endif
