#include "../vulkan.hpp"

#include <SDL_vulkan.h>
#include <iostream>
#include <cstring>

////
// NOTE: Comment this line out to disable the request for validation layers.
#define ENABLE_VALIDATION_LAYERS

#ifdef ENABLE_VALIDATION_LAYERS

////
// std::vector<const char *> validationLayers
//
// A list of validation layers to request.
std::vector<const char *> validationLayers {
    "VK_LAYER_LUNARG_standard_validation"
};

////
// bool checkValidationLayerSupport()
//
// Checks if all of the layers requested via validationLayers are supported on
// the current platform.
static bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName: validationLayers) {
        bool layerFound = false;
        for (auto& availableLayer: availableLayers) {
            if (strcmp(layerName, availableLayer.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t obj,
        size_t location,
        int32_t code,
        const char *layerPrefix,
        const char *msg,
        void *userData) {
    std::cerr << "Validation Layer: " << msg << std::endl;
    return VK_FALSE;
}
#endif

////
// std::vector<const char *> getRequiredExtensions(wfn_eng::sdl::Window&)
//
// Returns the required set of extensions necessary for the
static std::vector<const char *> getRequiredExtensions(wfn_eng::sdl::Window& window, bool vlSupp) {
    uint32_t sdlExtCount;
    SDL_Vulkan_GetInstanceExtensions(window.ref(), &sdlExtCount, nullptr);

    const char **pNames = new const char *[sdlExtCount];
    SDL_Vulkan_GetInstanceExtensions(window.ref(), &sdlExtCount, pNames);

    std::vector<const char *> exts(pNames, pNames + sdlExtCount);

    #ifdef ENABLE_VALIDATION_LAYERS
    if (vlSupp)
        exts.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    #endif

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
        #ifdef ENABLE_VALIDATION_LAYERS
        bool _debugSupport = checkValidationLayerSupport();
        if (!_debugSupport) {
            std::cout << "Validation layers were requested but are unsupported:" << std::endl;
            for (auto& str: validationLayers)
                std::cout << "  - " << str << std::endl;
        }
        #endif

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
        #ifdef ENABLE_VALIDATION_LAYERS
        if (_debugSupport) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }

        auto reqExts = getRequiredExtensions(window, _debugSupport);
        #else
        auto reqExts = getRequiredExtensions(window, false);
        #endif

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

        #ifdef ENABLE_VALIDATION_LAYERS
        if (_debugSupport) {
            VkDebugReportCallbackCreateInfoEXT callbackInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
                .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
                .pfnCallback = debugCallback
            };

            auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
            if (func == nullptr) {
                throw WfnError(
                    "wfn_eng::vulkan::Base",
                    "Constructor",
                    "Failed to create debug function for validation layer"
                );
            } else
                func(_instance, &callbackInfo, nullptr, &_debugCallback);
        }
        #endif
    }

    ////
    // ~Base()
    //
    // Destroying the VkInstance and VkSurfaceKHR.
    Base::~Base() {
        #ifdef ENABLE_VALIDATION_LAYERS
        if (_debugSupport) {
            auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");
            if (func != nullptr)
                func(_instance, _debugCallback, nullptr);
        }
        #endif

        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyInstance(_instance, nullptr);
    }

    ////
    // VkInstance instance()
    //
    // Provides access to the VkInstance.
    VkInstance& Base::instance() { return _instance; }

    ////
    // VkSurfaceKHR surface()
    //
    // Provides access to the VkSurfaceKHR.
    VkSurfaceKHR& Base::surface() { return _surface; }
}
