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
    // Core *_core;
    //
    // An (initially null) reference to the core.
    Core *Core::_core = nullptr;

    ////
    // Core(wff_eng::sdl::Window&, bool)
    //
    // Constructs the Base, Device, and Swapchain, given reference to an SDL
    // window wrapper.
    Core::Core(wfn_eng::sdl::Window& window, bool debugging) {
        _base = new Base(window, debugging);
        _device = new Device(base());
        _swapchain = new Swapchain(base(), device());
        _commandPools = new CommandPools(base(), device());
    }

    ////
    // ~Core()
    //
    // Handles the destruction of the Base, Device, and Swapchain.
    Core::~Core() {
        delete _commandPools;
        delete _swapchain;
        delete _device;
        delete _base;
    }

    ////
    // Core& instance()
    //
    // Provides a reference to the current instance of the Core object.
    Core& Core::instance() {
        if (Core::_core == nullptr) {
            throw WfnError(
                "wfn_eng::vulkan::Core",
                "instance",
                "Cannot access null Core instance"
            );
        }

        return *Core::_core;
    }

    ////
    // void destroy()
    //
    // Destroys the instance of the Core& object.
    void Core::destroy() {
        if (Core::_core == nullptr) {
            throw WfnError(
                "wfn_eng::vulkan::Core",
                "destroy",
                "Cannot destroy null Core instance"
            );
        }

        delete Core::_core;
        Core::_core = nullptr;
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

    ////
    // CommandPools& commandPools();
    //
    // Returns the CommandPools reference.
    CommandPools& Core::commandPools() { return *_commandPools; }

    ////
    // void initialize(sdl::Window, bool)
    //
    // Initializes the Core given reference to an SDL window with the option
    // for debugging.
    void Core::initialize(sdl::Window& window, bool debugging) {
        if (_core != nullptr) {
            throw WfnError(
                "wfn_eng::vulkan::Core",
                "initialize",
                "Cannot reinitialize Core"
            );
        }

        _core = new Core(window, debugging);
    }

    ////
    // void initialize(sdl::Window)
    //
    // Initializes the Core given reference to an SDL window.
    void Core::initialize(sdl::Window& window) {
        initialize(window, false);
    }
}
