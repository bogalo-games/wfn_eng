#include "../vulkan.hpp"

#include <SDL_vulkan.h>

////
// std::vector<const char *> getRequiredExtensions(wfn_eng::sdl::Window&)
//
// Returns the required set of extensions necessary for the
static std::vector<const char *> getRequiredExtensions(wfn_eng::sdl::Window& window) {
    uint32_t sdlExtCount;
    SDL_Vulkan_GetInstanceExtensions(window.ref(), &sdlExtCount, nullptr);

    const char **pNames = new const char *[sdlExtCount];
    SDL_Vulkan_GetInstanceExtensions(window.ref(), &sdlExtCount, pNames);

    std::vector<const char *> exts(pNames, pNames + sdlExtCount);

    // TODO: Add any additional extensions?

    return exts;
}

namespace wfn_eng::vulkan {
    ////
    // class Base
    //
    // A container for the base-level features of Vulkan, aka the VkInstance and
    // the VkSurfaceKHR.

    ////
    // Base(sdl::Window&)
    //
    // Construct the base of the Vulkan instance (VkInstance and
    // VkSurfaceKHR) from an SDL window wrapper.
    Base::Base(sdl::Window& window) {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "We Fight Now";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "wfn_eng";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto reqExts = getRequiredExtensions(window);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(reqExts.size());
        createInfo.ppEnabledExtensionNames = reqExts.data();

        if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::Base",
                "Constructor",
                "Create Instance"
            );
        }

        if (!SDL_Vulkan_CreateSurface(window.ref(), _instance, &_surface)) {
            throw WfnError(
                "wfn_eng::vulkan::Base",
                "Constructor",
                "Create Surface"
            );
        }
    }


    ////
    // ~Base()
    //
    // Destroying the VkInstance and VkSurfaceKHR.
    Base::~Base() {
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyInstance(_instance, nullptr);
    }

    ////
    // VkInstance instance()
    //
    // Provides access to the VkInstance.
    VkInstance Base::instance() { return _instance; }

    ////
    // VkSurfaceKHR surface()
    //
    // Provides access to the VkSurfaceKHR.
    VkSurfaceKHR Base::surface() { return _surface; }
}
