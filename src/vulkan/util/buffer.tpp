#include <cstdlib>

namespace wfn_eng::vulkan::util {
    ////
    // void copy_from(T)
    //
    // Copies the contents of the provided type into the buffer.
    template<typename T>
    void Buffer::copy_from(T value) {
        void *data;
        map(&data);
        memcpy(data, value, (size_t)size);
        unmap();
    }

    ////
    // void indirect_copy_from(Device&, VkCommandPool, T)
    //
    // Copies information from the provided type into the buffer via
    // copy_from and copy_to.
    template <typename T>
    void Buffer::indirect_copy_from(T value) {
        Buffer stagingBuffer(
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            Core::instance().device().requiredSharingMode()
        );

        stagingBuffer.copy_from(value);
        stagingBuffer.copy_to(*this);
    }
}
