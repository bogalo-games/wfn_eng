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

    ////
    // struct Buffer
    //
    // Provides a wrapper around Vulkan's buffers to be more C++ like.
    //
    // TODO: Find a better way to decentralize destruction, like in ~Buffer(),
    //       without having to maintain a bunch of references to the Device.
    struct Buffer {
        Device& _device;

        ////
        // VkDeviceSize size
        //
        // The size of the buffer.
        VkDeviceSize size;

        ////
        // VkBuffer handle
        //
        // The handle to the buffer itself.
        VkBuffer handle;

        ////
        // VkDeviceMemory memory
        //
        // The handle to the buffer's memory region.
        VkDeviceMemory memory;

        ////
        // Buffer(Device&, VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkSharingMode)
        //
        // Constructing a buffer, given the device, the size, usage flags, property
        // flags, and sharing mode.
        Buffer(Device&, VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkSharingMode);

        ////
        // ~Buffer()
        //
        // Destroying the buffer.
        ~Buffer();

        ////
        // void map(Device&, void **)
        //
        // Maps the data in this buffer (if able) to the provided memory range.
        // Will fail if the buffer is not accessible from the CPU.
        void map(Device&, void **);

        ////
        // void unmap(Device&)
        //
        // Unmaps the above mapped data.
        void unmap(Device&);

        ////
        // void copy_to(Device&, VkCommandPool, Buffer&, VkDeviceSize, VkDeviceSize, VkDeviceSize)
        //
        // Copies the contents of this buffer to another buffer. Will halt
        // during the copy. Provides the option to specify source offset,
        // destination offset, and copy size.
        void copy_to(Device&, VkCommandPool, Buffer&, VkDeviceSize, VkDeviceSize, VkDeviceSize);

        ////
        // void copy_to(Device&, VkCommandPool, Buffer&)
        //
        // Copies the contents of this buffer to another buffer. Will halt
        // during the copy.
        void copy_to(Device&, VkCommandPool, Buffer&);

        ////
        // void copy_from(Device&, T)
        //
        // Copies the contents of the provided type into the buffer. Will fail
        // if the bufer is not accessible from the CPU.
        template <typename T>
        void copy_from(Device&, T);

        ////
        // void indirect_copy_from(Device&, VkCommandPool, T)
        //
        // Copies information from the provided type into the buffer via
        // copy_from and copy_to.
        template <typename T>
        void indirect_copy_from(Device&, VkCommandPool, T);


        // Rule of 5's.
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) = delete;
    };
}

#include "util/buffer.tpp"

#endif
