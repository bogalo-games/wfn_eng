#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <functional>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <set>

const int WIDTH  = 640;
const int HEIGHT = 480;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

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
// glfwErrorHandler
//
// Prints errors raised by GLFW.
static void glfwErrorHandler(int errCode, const char *desc) {
    std::cerr << "GLFW Error (" << errCode << "): " << desc << std::endl;
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
    void makeInstance() {
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
 
        // Getting the set of required GLFW extensions.
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        if (glfwExtensionCount == 0 && glfwExtensions == nullptr)
            throw std::runtime_error("Failed to collect required GLFW extensions.");

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        // Adding validation layers
        if (enableValidationLayer) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else
            createInfo.enabledLayerCount = 0;

        // Printing the available extensions, for fun and profit.
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::cout << "Available extensions:" << std::endl;
        for (const auto& ext: extensions)
            std::cout << "  " << ext.extensionName << std::endl;
        std::cout << std::endl;

        // Constructing the VkInstance
        VkResult result;
        if ((result = vkCreateInstance(&createInfo, nullptr, &instance)) != VK_SUCCESS) {
            std::cerr << result << std::endl;
            throw std::runtime_error("Failed to create instance.");
        }
    }

    VkInstance instance;

    Instance() {
        makeInstance();
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
// Physical
//
// Utility class for choosing a VkPhysicalDevice.
struct Physical {
    /////
    // isDeviceSuitable
    //
    // Decides if a GPU is suitable.
    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        QueueFamily family(device, surface);
        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            family.isComplete();
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
//
struct Surface {
    VkInstance instance;
    VkSurfaceKHR surface;

    void createSurface(VkInstance instance, GLFWwindow* window) {
        VkResult result;
        if ((result = glfwCreateWindowSurface(instance, window, nullptr, &surface)) != VK_SUCCESS) {
            std::cerr << "Create surface result: " << result << std::endl;
            throw std::runtime_error("Failed to create surface");
        }
    }

    Surface(VkInstance instance, GLFWwindow* window) {
        this->instance = instance;
        createSurface(instance, window);
    }

    ~Surface() {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }
};

class HelloTriangleApplication {
private:
    GLFWwindow *window;

    VkDebugReportCallbackEXT callback;

    Instance *instance;
    Physical *physical;
    Logical *logical;
    Surface *surface;

    /////
    // GLFW
    void initWindow() {
        glfwSetErrorCallback(&glfwErrorHandler);

        if (glfwInit() == GLFW_FALSE)
            throw std::runtime_error("Failed to initialize GLFW.");
        if (glfwVulkanSupported() == GLFW_FALSE)
            throw std::runtime_error("GLFW does not support Vulkan on this machine.");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Test", nullptr, nullptr);
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
        instance = new Instance();
        initDebug();

        surface = new Surface(instance->instance, window);
        physical = new Physical(instance->instance, surface->surface);
        logical = new Logical(physical->physical, surface->surface);
    }

    ////
    // Game Logic
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    ////
    // Cleaning Up
    void cleanup() {

        delete surface;
        delete logical;
        delete physical;

        if (enableValidationLayer)
            DestroyDebugReportCallbackEXT(instance->instance, callback, nullptr);
        delete instance;

        glfwDestroyWindow(window);
        glfwTerminate();
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
    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);

    std::cout << "GLFW Info:" << std::endl;
    std::cout << "  GLFW (Header):  " << GLFW_VERSION_MAJOR << "."
                                      << GLFW_VERSION_MINOR << "."
                                      << GLFW_VERSION_REVISION << std::endl;
    std::cout << "  GLFW (Library): " << major << "." << minor << "." << rev << std::endl;
    std::cout << "  GLFW (String):  " << glfwGetVersionString() << std::endl;
    std::cout << std::endl;

    HelloTriangleApplication app;
    try {
        app.run();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
