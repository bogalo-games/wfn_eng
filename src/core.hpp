#ifndef __WFN_ENG_CORE_HPP__
#define __WFN_ENG_CORE_HPP__

#include <SDL.h>

namespace wfn_eng {
    ////
    // CoreConfig
    //
    // A struct that provides configuration to the Core class.
    struct CoreConfig {
        // TODO: Define CoreConfig
    };

    static CoreConfig DEFAULT_CONFIG = {
        // TODO: Implement DEFAULT_CONFIG
    };

    class Core {
    public:
        VkInstance instance;

        VkPhysicalDevice physicalDevice;
        VkPhysicalDeviceProperties physicalDeviceProperties;
        VkPhysicalDeviceFeatures physicalDeviceFeatures;

        VkDevice logicalDevice;
        VkQueue graphicsQueue;

        VkSurfaceKHR surface;
        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR surfacePresentMode;
        VkExtent2D surfaceExtent;

        VkSwapChainKHR swapChain;
        std::vector<Image> swapChainImages;
        std::vector<ImageViews> imageViews;

        ////
        // Core
        //
        // Constructs the Vulkan instance, given the reference to a SDL_Window
        // and an optional CoreConfig. If no CoreConfig is provided, uses
        // DEFAULT_CONFIG instead.
        Core(SDL_Window *, CoreConfig);
        Core(SDL_Window *);

        ////
        // ~Core
        //
        // Frees all of the resources allocated by Vulkan.
        ~Core();
    };
}

#endif
