#ifndef __WFN_ENG_ENGINE_UTIL_HPP__
#define __WFN_ENG_ENGINE_UTIL_HPP__

#include <vulkan/vulkan.h>

namespace wfn_eng::engine::util {
    ////
    // struct Buffer
    //
    // Provides a wrapper around Vulkan's buffers to be more C++ like.
    struct Buffer {
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
        Buffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkSharingMode);

        ////
        // ~Buffer()
        //
        // Destroying the buffer.
        ~Buffer();

        ////
        // void map(void **)
        //
        // Maps the data in this buffer (if able) to the provided memory range.
        // Will fail if the buffer is not accessible from the CPU.
        void map(void **);

        ////
        // void unmap()
        //
        // Unmaps the above mapped data.
        void unmap();

        ////
        // void copy_to(Buffer&, VkDeviceSize, VkDeviceSize, VkDeviceSize)
        //
        // Copies the contents of this buffer to another buffer. Will halt
        // during the copy. Provides the option to specify source offset,
        // destination offset, and copy size.
        void copy_to(Buffer&, VkDeviceSize, VkDeviceSize, VkDeviceSize);

        ////
        // void copy_to(Buffer&)
        //
        // Copies the contents of this buffer to another buffer. Will halt
        // during the copy.
        void copy_to(Buffer&);

        ////
        // void copy_from(T)
        //
        // Copies the contents of the provided type into the buffer. Will fail
        // if the bufer is not accessible from the CPU.
        template <typename T>
        void copy_from(T);

        ////
        // void indirect_copy_from(T)
        //
        // Copies information from the provided type into the buffer via
        // copy_from and copy_to.
        template <typename T>
        void indirect_copy_from(T);


        // Rule of 5's.
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) = delete;
    };
}

#include "util/buffer.tpp"

#endif
