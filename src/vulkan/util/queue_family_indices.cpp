#include "../util.hpp"

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

        for (int i = 0; i < queueFamilies.size(); i++) {
            // Selecting the graphics family
            if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphicsFamily = i;

            // Selecting the presentation family
            VkBool32 presentationSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);
            if (queueFamilies[i].queueCount > 0 && presentationSupport)
                presentationFamily = i;

            // Selecting the transfer family
            if (queueFamilies[i].queueCount > 0 &&
                queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {

                transferFamily = i;
            }

            if (sufficient())
                break;
        }

        if (!sufficient()) {
            throw WfnError(
                "wfn_end::vulkan::util::QueueFamilyIndices",
                "QueueFamilyIndices",
                "Could not find a suitable set of queue indices"
            );
        }
    }

    ////
    // QueueFamilyIndices(Base&, Device&)
    //
    // Given the reference to a Base and a Device, query the relevant
    // queue indices.
    QueueFamilyIndices::QueueFamilyIndices(Base& base, Device& device) :
            QueueFamilyIndices(base.surface(), device.physical()) { }

    ////
    // bool sufficient
    //
    // Checks if the queue family's indices are sufficient for use in
    // the rest of the program.
    bool QueueFamilyIndices::sufficient() {
        return graphicsFamily >= 0 &&
               presentationFamily >= 0 &&
               transferFamily >= 0;
    }
}
