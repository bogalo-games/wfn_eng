namespace wfn_eng::vulkan::util {
    ////
    // void copy_from(Device&, T&)
    //
    // Copies the contents of the provided type into the buffer.
    template<typename T>
    void Buffer::copy_from(Device& device, T value) {
        void *data;
        map(device, &data);
        memcpy(data, value, (size_t)size);
        unmap(device);
    }
}
