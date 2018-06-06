#include "../vulkan.hpp"

#include <iostream>
#include <set>

#include "util.hpp"

////
// bool checkExtensionSupport(VkPhysicalDevice)
//
// Checking if a VkPhysicalDevice supports the required extensions.
static bool checkExtensionSupport(wfn_eng::vulkan::Base& base, VkPhysicalDevice physical) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physical, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physical, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(base.deviceExtensions.begin(), base.deviceExtensions.end());
    for (const auto& extension: availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

////
// bool suitable(VkPhysicalDevice)
//
// Decides if a VkPhysicalDevice is suitable.
static bool suitable(wfn_eng::vulkan::Base& base, VkPhysicalDevice physical) {
    wfn_eng::vulkan::util::QueueFamilyIndices indices(base.surface(), physical);

    bool extensionsSupported = checkExtensionSupport(base, physical);
    bool swapchainAdequate = false;
    if (extensionsSupported) {
        wfn_eng::vulkan::util::SwapchainSupport swapchainSupport(base.surface(), physical);
        swapchainAdequate = swapchainSupport.sufficient();
    }

    return indices.sufficient() && extensionsSupported && swapchainAdequate;
}

namespace wfn_eng::vulkan {
    ////
    // class Device
    //
    // A container for the device-related features of Vulkan, that includes the
    // physical and logical devices, along with their relevant queues.

    ////
    // makePhysicalDevice
    //
    // Constructs the VkPhysicalDevice.
    void Device::makePhysicalDevice(Base& base) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(base.instance(), &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw WfnError(
                "wfn_eng::Vulkan::Device",
                "makePhysicalDevice",
                "No GPUs"
            );
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(base.instance(), &deviceCount, devices.data());

        for (const auto& device: devices) {
            if (suitable(base, device)) {
                _physical = device;
                break;
            }
        }

        if (_physical == VK_NULL_HANDLE) {
            throw WfnError(
                "wfn_eng::vulkan::Device",
                "makePhysicalDevice",
                "No suitable GPUs"
            );
        }

        if (base.debuggingEnabled()) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(_physical, &props);
            std::cout << "Selected: " << props.deviceName << std::endl;
        }
    }

    ////
    // makeLogicalDevice
    //
    // Constructs the VkDevice, along with both the graphics and
    // presentation queues.
    void Device::makeLogicalDevice(Base& base) {
        wfn_eng::vulkan::util::QueueFamilyIndices indices(base.surface(), physical());

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<int> uniqueQueueFamilies = {
            indices.graphicsFamily,
            indices.presentationFamily,
            indices.transferFamily
        };

        float queuePriority = 1.0f;
        for (int queueFamily: uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(base.deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = base.deviceExtensions.data();

        if (vkCreateDevice(physical(), &createInfo, nullptr, &_logical) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::Device",
                "makeLogicalDevice",
                "Create Device"
            );
        }

        _graphicsQueueIndex = indices.graphicsFamily;
        _presentationQueueIndex = indices.presentationFamily;
        _transferQueueIndex = indices.transferFamily;

        vkGetDeviceQueue(
            logical(),
            indices.graphicsFamily,
            0,
            &_graphicsQueue
        );

        vkGetDeviceQueue(
            logical(),
            indices.presentationFamily,
            0,
            &_presentationQueue
        );

        vkGetDeviceQueue(
            logical(),
            indices.transferFamily,
            0,
            &_transferQueue
        );
    }

    ////
    // Device(Base&)
    //
    // Constructing a device from a Base.
    Device::Device(Base& base) {
        makePhysicalDevice(base);
        makeLogicalDevice(base);
    }

    ////
    // ~Device()
    //
    // Destroying the Device.
    Device::~Device() {
        vkDestroyDevice(logical(), nullptr);
    }

    ////
    // VkPhysicalDevice physical()
    //
    // Getting the reference to the VkPhysicalDevice.
    VkPhysicalDevice& Device::physical() { return _physical; }

    ////
    // VkDevice logical()
    //
    // Getting the reference to the VkDevice.
    VkDevice& Device::logical() { return _logical; }

    ////
    // VkQueue graphicsQueue()
    //
    // Getting the graphics queue.
    VkQueue& Device::graphicsQueue() { return _graphicsQueue; }

    ////
    // VkQueue presentationQueue()
    //
    // Getting the presentation queue.
    VkQueue& Device::presentationQueue() { return _presentationQueue; }

    ////
    // VkQueue transferQueue()
    //
    // Getting the transfer queue.
    VkQueue& Device::transferQueue() { return _transferQueue; }

    ////
    // VkSharingMode requiredSharingMode()
    //
    // Returns the required sharing mode, depending on whether the graphics
    // and transfer queues are separate.
    VkSharingMode Device::requiredSharingMode() {
        if (_graphicsQueueIndex == _transferQueueIndex)
            return VK_SHARING_MODE_EXCLUSIVE;
        else
            return VK_SHARING_MODE_CONCURRENT;
    }
}
