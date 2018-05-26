#include <cstdlib>

namespace wfn_eng::vulkan::util {
    ////
    // void copy_from(Device&, T)
    //
    // Copies the contents of the provided type into the buffer.
    template<typename T>
    void Buffer::copy_from(Device& device, T value) {
        void *data;
        map(device, &data);
        memcpy(data, value, (size_t)size);
        unmap(device);
    }

    ////
    // void indirect_copy_from(Device&, VkCommandPool, T)
    //
    // Copies information from the provided type into the buffer via
    // copy_from and copy_to.
    template <typename T>
    void Buffer::indirect_copy_from(Device& device, VkCommandPool transferPool, T value) {
        Buffer stagingBuffer(
            device,
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            VK_SHARING_MODE_CONCURRENT
        );

        stagingBuffer.copy_from(device, value);
        stagingBuffer.copy_to(device, transferPool, *this);
    }
}
