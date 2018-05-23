#include "../vulkan.hpp"

namespace wfn_eng::vulkan {
    ////
    // Core
    //
    // A wrapper around the Base, Device, and Swapchain implementations to
    // provide a central wrapper around the core functionality of Vulkan.
    //
    // Does not include reference to graphics pipelines or command buffers
    // because they are application specific.

    ////
    // Core(wnf_eng::sdl::Window&)
    //
    // Constructs the Base, Device, and Swapchain, given reference to an SDL
    // window wrapper.
    Core::Core(wfn_eng::sdl::Window& window) {
        _base = new Base(window);
        _device = new Device(base());
        _swapchain = new Swapchain(base(), device());
    }

    ////
    // ~Core()
    //
    // Handles the destruction of the Base, Device, and Swapchain.
    Core::~Core() {
        delete _swapchain;
        delete _device;
        delete _base;
    }

    ////
    // Base& base()
    //
    // Returns the Base reference.
    Base& Core::base() { return *_base; }

    ////
    // Device& device()
    //
    // Returns the Device reference.
    Device& Core::device() { return *_device; }

    ////
    // Swapchain& swapchain()
    //
    // Returns the Swapchain reference.
    Swapchain& Core::swapchain() { return *_swapchain; }
}
