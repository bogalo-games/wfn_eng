#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include <SDL.h>
#include <SDL_vulkan.h>

#include <functional>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <set>

const int WIDTH  = 640;
const int HEIGHT = 480;

#define DEBUG

#define NDEBUG
#ifdef NDEBUG
const bool enableValidationLayer = false;
#else
const bool enableValidationLayer = true;
#endif

////
// debugCallback
//
// The debug callback for the provided validation layer.
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t obj,
        size_t location,
        int32_t code,
        const char *layerPrefix,
        const char *msg,
        void *userData) {
    std::cerr << "Validation layer: " << msg << std::endl;

    return VK_FALSE;
}

////
// CreateDebugReportCallbackEXT
//
// Used to register the Vulkan validation layer with the application.
VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

////
// DestroyDEbugReportCallbackEXT
//
// Used to remove the Vulkan validation layer from the application.
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(instance, callback, pAllocator);
    }
}

////
// Instance
//
// Utility class for creating a VkInstance.
struct Instance {
    const std::vector<const char *> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    ////
    // checkValidationLayerSupport
    //
    // Checks if the application supports the required validation layers.
    bool checkValidationLayerSupport() {
        if (!enableValidationLayer)
            return true;

        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName: validationLayers) {
            bool layerFound = false;
            std::cout << "Searching for: " << layerName << std::endl;
            for (const auto& layerProperties: availableLayers) {
                std::cout << "  " << layerProperties.layerName << std::endl;
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                std::cout << std::endl;
                return false;
            }
        }

        std::cout << std::endl;
        return true;
    }

    ////
    // makeInstance
    //
    // Creates a VkInstance.
    void makeInstance(SDL_Window *window) {
        if (!checkValidationLayerSupport())
            throw std::runtime_error("Validation layers requested but not found.");

        // Constructing application info.
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan Test";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // Constructing instance construction info.
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
        createInfo.pApplicationInfo = &appInfo;

        // Registering ALL the extensions!
        uint32_t extensionCount;
        if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr))
            throw std::runtime_error("Could not get required extension count");

        const char **pNames = new const char *[extensionCount];
        if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, pNames))
            throw std::runtime_error("Could not get extensions.");

        createInfo.enabledExtensionCount = extensionCount;
        createInfo.ppEnabledExtensionNames = pNames;

        // Adding validation layers
        if (enableValidationLayer) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else
            createInfo.enabledLayerCount = 0;


        // Constructing the VkInstance
        VkResult result;
        if ((result = vkCreateInstance(&createInfo, nullptr, &instance)) != VK_SUCCESS) {
            std::cerr << result << std::endl;
            throw std::runtime_error("Failed to create instance.");
        }

        delete[] pNames;
    }

    VkInstance instance;

    Instance(SDL_Window *window) {
        makeInstance(window);
    }

    ~Instance() {
        vkDestroyInstance(instance, nullptr);
    }
};

////
// QueueFamily
//
// A utility class for constructing a QueueFamily.
struct QueueFamily {
    int graphicsFamily = -1;
    int presentFamily = -1;

    void findValidQueueFamily(VkPhysicalDevice device, VkSurfaceKHR surface) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (int i = 0; i < queueFamilyCount; i++) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphicsFamily = i;

            if (queueFamilies[i].queueCount > 0 && presentSupport)
                presentFamily = i;

            if (isComplete())
                break;
        }
    }

    QueueFamily(VkPhysicalDevice device, VkSurfaceKHR surface) {
        graphicsFamily = -1;
        presentFamily = -1;
        findValidQueueFamily(device, surface);
    }

    bool isComplete() { return graphicsFamily >= 0 && presentFamily >= 0; }
};

////
// SwapChainSupport
//
// Provides information on the supported features for a VkPhysicalDevice.
struct SwapChainSupport {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    SwapChainSupport(VkPhysicalDevice physical, VkSurfaceKHR surface) {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &formatCount, nullptr);
        if (formatCount > 0) {
            formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &formatCount, formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &presentModeCount, nullptr);
        if (presentModeCount > 0) {
            presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &presentModeCount, presentModes.data());
        }
    }

    ////
    // sufficient
    //
    // Checks whether the capabilities, formats, and presentModes are
    // sufficient.
    bool sufficient() {
#ifdef DEBUG
        std::cout << "  Formats:       " << formats.size() << std::endl;
        std::cout << "  Present Modes: " << presentModes.size() << std::endl;
#endif
        return !formats.empty() && !presentModes.empty();
    }
};

////
// Physical
//
// Utility class for choosing a VkPhysicalDevice.
struct Physical {
    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    ////
    // areExtensionsSupported
    //
    // Returns whether or not a device supports a given set of extensions.
    bool areExtensionsSupported(VkPhysicalDevice device) {
        uint32_t supExtCt;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &supExtCt, nullptr);

        std::vector<VkExtensionProperties> supExt(supExtCt);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &supExtCt, supExt.data());

#ifdef DEBUG
        std::cout << "  Exts: " << std::endl;
#endif

        std::set<std::string> reqExt(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto& ext: supExt) {
#ifdef DEBUG
            std::cout << "    " << ext.extensionName << std::endl;
#endif

            reqExt.erase(ext.extensionName);
        }

        return reqExt.empty();
    }

    /////
    // isDeviceSuitable
    //
    // Decides if a GPU is suitable.
    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

#ifdef DEBUG
        std::cout << "Device: " << deviceProperties.deviceName << std::endl;
#endif

        bool extSupport = areExtensionsSupported(device);
        bool swapSupport = false;
        if (extSupport) {
            SwapChainSupport support(device, surface);
            swapSupport = support.sufficient();
        }

        QueueFamily family(device, surface);
        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            family.isComplete() &&
            extSupport &&
            swapSupport;
    }

    ////
    // pickPhysicalDevice
    //
    // Picks a physical device to use.
    void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
        physical = VK_NULL_HANDLE;
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            throw std::runtime_error("No physical devices found");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const VkPhysicalDevice& device: devices) {
            if (isDeviceSuitable(device, surface)) {
                physical = device;
                break;
            }
        }

        if (physical == VK_NULL_HANDLE)
            throw std::runtime_error("Failed to find a GPU");
    }

    VkPhysicalDevice physical;
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures features;

    Physical(VkInstance instance, VkSurfaceKHR surface) {
        pickPhysicalDevice(instance, surface);
    }
};

////
// Logical
//
// Container for the logical device and graphicsQueue.
struct Logical {
    QueueFamily* family;
    VkDevice device;
    VkQueue graphicsQueue;

    ////
    // makeDevice
    //
    // Constructs a new VkDevice.
    void makeDevice(VkPhysicalDevice physical) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = family->graphicsFamily;
        queueCreateInfo.queueCount = 1;

        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = 0;

        VkResult result;
        if ((result = vkCreateDevice(physical, &createInfo, nullptr, &device)) != VK_SUCCESS) {
            std::cerr << "Create device result: " << result << std::endl;
            throw std::runtime_error("Failed to create device");
        }

        vkGetDeviceQueue(device, family->graphicsFamily, 0, &graphicsQueue);
    }

    Logical(VkPhysicalDevice physical, VkSurfaceKHR surface) {
        family = new QueueFamily(physical, surface);
        makeDevice(physical);
    }

    ~Logical() {
        delete family;
        vkDestroyDevice(device, nullptr);
    }
};

////
// Surface
//
// Provides a surface for the... thing to do a thing.
struct Surface {
    VkInstance instance;
    VkSurfaceKHR surface;

    void createSurface(VkInstance instance, SDL_Window *window) {
        if (!SDL_Vulkan_CreateSurface(window, instance, &surface))
            throw std::runtime_error("Failed to build surface");
    }

    Surface(VkInstance instance, SDL_Window* window) {
        this->instance = instance;
        createSurface(instance, window);
    }

    ~Surface() {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }
};

////
// SwapChain
//
// Builds the SwapChain!
struct SwapChain {
    ////
    // chooseFormat
    //
    // Chooses a VkSurfaceFormatKHR given the set of available formats.
    VkSurfaceFormatKHR chooseFormat() {
        // If there is no preferred format, choose the best one.
        if (support.formats.size() == 1 && support.formats[0].format == VK_FORMAT_UNDEFINED) {
            return {
                VK_FORMAT_B8G8R8A8_UNORM,
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            };
        }

        // See if the best format is in the set of preferred formats.
        for (const auto& format: support.formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return format;
        }

        // Choose any old format.
        return support.formats[0];
    }

    ////
    // presentModeRanking
    //
    // Provides a ranking for VkPresentModeKHR such that we can choose the best
    // one!
    int presentModeRanking(VkPresentModeKHR presentMode) {
        switch (presentMode) {
        case VK_PRESENT_MODE_MAILBOX_KHR:
            return 3;
        case VK_PRESENT_MODE_FIFO_KHR:
            return 2;
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
            return 1;
        default:
            return 0;
        };
    }

    ////
    // choosePresentMode
    //
    // Chooses the VkPresentModeKHR given the set of available presentation
    // modes.
    VkPresentModeKHR choosePresentMode() {
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
        int bestModeRanking = -1;

        for (const auto& presentMode: support.presentModes) {
            int n = presentModeRanking(presentMode);
            if (n > bestModeRanking) {
                bestMode = presentMode;
                bestModeRanking = n;
            }
        }

        return bestMode;
    }

    ////
    // chooseExtent
    //
    // Chooses the extent that best matches the extents of the SDL_Window.
    VkExtent2D chooseExtent() {
        if (support.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return support.capabilities.currentExtent;
        else {
            VkExtent2D ext = { WIDTH, HEIGHT };

            ext.width = std::max(
                support.capabilities.minImageExtent.width,
                std::min(
                    support.capabilities.maxImageExtent.width,
                    ext.width
                )
            );

            ext.height = std::max(
                support.capabilities.minImageExtent.height,
                std::min(
                    support.capabilities.maxImageExtent.height,
                    ext.height
                )
            );

            return ext;
        }
    }

    void makeSwapChain(VkDevice device, VkSurfaceKHR surface) {
        uint32_t imageCount = support.capabilities.minImageCount + 1;
        if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount)
            imageCount = support.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = format.format;
        createInfo.imageColorSpace = format.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        VkResult result;
        if ((result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain)) != VK_SUCCESS) {
          std::cerr << "Swapchain result: " << result << std::endl;
          throw std::runtime_error("Failed to create swapchain");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
    }

    SwapChainSupport support;

    VkSurfaceFormatKHR format;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;

    VkDevice device;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;

    SwapChain(VkPhysicalDevice physical, VkDevice device, VkSurfaceKHR surface) :
            support(physical, surface) {
        format = chooseFormat();
        presentMode = choosePresentMode();
        extent = chooseExtent();
        makeSwapChain(device, surface);

        this->device = device;
    }

    ~SwapChain() {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }
};

struct ImageViews {
    VkDevice device;
    std::vector<VkImageView> imageViews;

    void makeImageViews(VkDevice device, SwapChain *swapChain) {
        imageViews.resize(swapChain->swapChainImages.size());
        for (size_t i = 0; i < swapChain->swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChain->swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChain->format.format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 1;
            createInfo.subresourceRange.layerCount = 1;

            VkResult result;
            if ((result = vkCreateImageView(device, &createInfo, nullptr, &imageViews[i])) != VK_SUCCESS) {
                std::cerr << "Image view " << i << " result: " << result << std::endl;
                throw std::runtime_error("Failed to create image view");
            }
        }
    }

    ImageViews(VkDevice device, SwapChain *swapChain) {
        makeImageViews(device, swapChain);
        this->device = device;
    }

    ~ImageViews() {
        for (auto imageView: imageViews)
            vkDestroyImageView(device, imageView, nullptr);
    }
};

class HelloTriangleApplication {
private:
    SDL_Window *window;

    VkDebugReportCallbackEXT callback;

    Instance *instance;
    Surface *surface;
    Physical *physical;
    Logical *logical;
    SwapChain *swapChain;
    ImageViews *imageViews;

    /////
    // GLFW
    void initWindow() {
        SDL_Init(SDL_INIT_EVERYTHING);
        SDL_Vulkan_LoadLibrary("vulkan/macOS/lib/libvulkan.1.dylib");
        window = SDL_CreateWindow(
            "Testing Vulkan",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            WIDTH,
            HEIGHT,
            SDL_WINDOW_VULKAN
        );
    }

    ////
    // Vulkan
    void initDebug() {
        if (!enableValidationLayer) return;

        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = debugCallback;

        VkResult result;
        if ((result = CreateDebugReportCallbackEXT(instance->instance, &createInfo, nullptr, &callback)) != VK_SUCCESS) {
            std::cerr << "Debug callback status: " << result << std::endl;
            throw std::runtime_error("Failed to create debug callback.");
        }
    }

    void initVulkan() {
        instance = new Instance(window);
        initDebug();

        surface = new Surface(instance->instance, window);
        physical = new Physical(instance->instance, surface->surface);
        logical = new Logical(physical->physical, surface->surface);
        swapChain = new SwapChain(physical->physical, logical->device, surface->surface);
        imageViews = new ImageViews(logical->device, swapChain);
    }

    ////
    // Game Logic
    void mainLoop() {
        bool quit = false;
        SDL_Event event;

        while (true) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT)
                    quit = true;
            }

            if (quit == true)
                break;
            SDL_Delay(16);
        }
    }

    ////
    // Cleaning Up
    void cleanup() {
        delete swapChain;
        delete surface;
        delete logical;
        delete physical;

        if (enableValidationLayer)
            DestroyDebugReportCallbackEXT(instance->instance, callback, nullptr);
        delete instance;

        SDL_DestroyWindow(window);
        SDL_Vulkan_UnloadLibrary();
        SDL_Quit();
    }

public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }
};

int main() {
    HelloTriangleApplication app;
    try {
        app.run();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
