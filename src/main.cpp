#include <vulkan/vulkan.h>
#include <SDL.h>
#include <SDL_vulkan.h>

#include <functional>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <set>

#include "vulkan.hpp"
#include "sdl.hpp"

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
// DestroyDebugReportCallbackEXT
//
// Used to remove the Vulkan validation layer from the application.
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(instance, callback, pAllocator);
    }
}

////
// readFile
//
// Utility function to load files into a vector of characters.
static std::vector<char> readFile(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("Failed to open file");

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
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
    VkQueue presentQueue;

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
        vkGetDeviceQueue(device, family->presentFamily, 0, &presentQueue);
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

struct GraphicsPipeline {
    const std::string vertPath = "src/shaders/vert.spv";
    const std::string fragPath = "src/shaders/frag.spv";

    VkShaderModule makeShader(VkDevice device, const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule module;
        VkResult result;
        if ((result = vkCreateShaderModule(device, &createInfo, nullptr, &module)) != VK_SUCCESS) {
            std::cerr << "Shader module result: " << result << std::endl;
            throw std::runtime_error("Failed to create shader module");
        }

        return module;
    }

    VkPipelineShaderStageCreateInfo shaderCreateInfo(VkShaderModule module, VkShaderStageFlagBits stage) {
        VkPipelineShaderStageCreateInfo createInfo = {};

        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage = stage;
        createInfo.module = module;
        createInfo.pName = "main";

        return createInfo;
    }

    void makeRenderPass(VkDevice device, SwapChain *swapChain) {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = swapChain->format.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void makePipeline(VkDevice device, SwapChain *swapChain) {
        VkShaderModule vertModule = makeShader(device, readFile(vertPath));
        VkShaderModule fragModule = makeShader(device, readFile(fragPath));

        auto vertCreateInfo = shaderCreateInfo(vertModule, VK_SHADER_STAGE_VERTEX_BIT);
        auto fragCreateInfo = shaderCreateInfo(fragModule, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkPipelineShaderStageCreateInfo shaderStages[] = {
            vertCreateInfo,
            fragCreateInfo
        };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChain->extent.width;
        viewport.height = (float)swapChain->extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = swapChain->extent;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        VkResult result;
        if ((result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout)) != VK_SUCCESS) {
            std::cerr << "Graphics pipeline layout result: " << result << std::endl;
            throw std::runtime_error("Failed to create graphics pipeline layout");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr; // Optional
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if ((result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline)) != VK_SUCCESS) {
            std::cerr << "Graphics pipeline result: " << result << std::endl;
            throw std::runtime_error("Failed to create graphics pipeline");
        }
    }

    VkDevice device;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    GraphicsPipeline(VkDevice device, SwapChain *swapChain) {
        makeRenderPass(device, swapChain);
        makePipeline(device, swapChain);

        this->device = device;
    }

    ~GraphicsPipeline() {
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
    }
};

struct FrameBuffers {
    VkDevice device;
    std::vector<VkFramebuffer> swapChainFrameBuffers;

    FrameBuffers(VkDevice device, SwapChain *swapChain, ImageViews *imageViews, GraphicsPipeline *graphicsPipeline) {
        swapChainFrameBuffers.resize(imageViews->imageViews.size());
        for (int i = 0; i < imageViews->imageViews.size(); i++) {
            VkImageView attachments[] = {
                imageViews->imageViews[i]
            };

            VkFramebufferCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.renderPass = graphicsPipeline->renderPass;
            createInfo.attachmentCount = 1;
            createInfo.pAttachments = attachments;
            createInfo.width = swapChain->extent.width;
            createInfo.height = swapChain->extent.height;
            createInfo.layers = 1;

            VkResult result;
            if ((result = vkCreateFramebuffer(device, &createInfo, nullptr, &swapChainFrameBuffers[i])) != VK_SUCCESS) {
                std::cerr << "Framebuffer " << i << " result: " << result << std::endl;
                throw std::runtime_error("Failed to create framebuffer");
            }
        }

        this->device = device;
    }

    ~FrameBuffers() {
        for (auto frameBuffer: swapChainFrameBuffers)
            vkDestroyFramebuffer(device, frameBuffer, nullptr);
    }
};

struct CommandBuffers {
    VkDevice device;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    void createCommandPool(VkDevice device, QueueFamily *queueFamily) {
        VkCommandPoolCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.queueFamilyIndex = queueFamily->graphicsFamily;
        createInfo.flags = 0;

        VkResult result;
        if ((result = vkCreateCommandPool(device, &createInfo, nullptr, &commandPool)) != VK_SUCCESS) {
            std::cerr << "Command pool result: " << result << std::endl;
            throw std::runtime_error("Failed to make command pool");
        }
    }

    void createCommandBuffers(SwapChain *swapChain, GraphicsPipeline *graphicsPipeline, FrameBuffers *frameBuffers) {
        commandBuffers.resize(frameBuffers->swapChainFrameBuffers.size());

        VkCommandBufferAllocateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        createInfo.commandPool = commandPool;
        createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        createInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        VkResult result;
        if ((result = vkAllocateCommandBuffers(device, &createInfo, commandBuffers.data())) != VK_SUCCESS) {
            std::cerr << "Command buffers result: " << result << std::endl;
            throw std::runtime_error("Failed to allocate command buffers");
        }

        for (size_t i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
                throw std::runtime_error("Failed to begin recording command buffer");

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = graphicsPipeline->renderPass;
            renderPassInfo.framebuffer = frameBuffers->swapChainFrameBuffers[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapChain->extent;

            VkClearValue clearColor = { 0.3f, 0.3f, 0.3f, 1.0f };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->pipeline);
            vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
            vkCmdEndRenderPass(commandBuffers[i]);

            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to record command buffer");
        }
    }

    CommandBuffers(VkDevice device, QueueFamily *queueFamily, SwapChain *swapChain, GraphicsPipeline *graphicsPipeline, FrameBuffers *frameBuffers) {
        createCommandPool(device, queueFamily);
        createCommandBuffers(swapChain, graphicsPipeline, frameBuffers);

        this->device = device;
    }

    ~CommandBuffers() {
        vkDestroyCommandPool(device, commandPool, nullptr);
    }
};

struct Semaphores {
    VkDevice device;
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;

    Semaphores(VkDevice device) {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailable) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinished) != VK_SUCCESS) {

            throw std::runtime_error("Failed to create semaphores");
        }

        this->device = device;
    }

    ~Semaphores() {
        vkDestroySemaphore(device, renderFinished, nullptr);
        vkDestroySemaphore(device, imageAvailable, nullptr);
    }
};

class HelloTriangleApplication {
private:
    wfn_eng::sdl::Window *window;

    VkDebugReportCallbackEXT callback;

    Instance *instance;
    Surface *surface;
    Physical *physical;
    Logical *logical;
    SwapChain *swapChain;
    ImageViews *imageViews;
    GraphicsPipeline *graphicsPipeline;
    FrameBuffers *frameBuffers;
    CommandBuffers *commandBuffers;
    Semaphores *semaphores;

    /////
    // GLFW
    void initWindow() {
        wfn_eng::sdl::WindowConfig cfg {
            .vulkanPath = "vulkan/macOS/lib/libvulkan.1.dylib",
            .windowName = "Testing Vulkan",
            .width = WIDTH,
            .height = HEIGHT,
            .flags = 0
        };

        window = new wfn_eng::sdl::Window(cfg);
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
        instance = new Instance(window->ref());
        initDebug();

        surface = new Surface(instance->instance, window->ref());
        physical = new Physical(instance->instance, surface->surface);
        logical = new Logical(physical->physical, surface->surface);
        swapChain = new SwapChain(physical->physical, logical->device, surface->surface);
        imageViews = new ImageViews(logical->device, swapChain);
        graphicsPipeline = new GraphicsPipeline(logical->device, swapChain);
        frameBuffers = new FrameBuffers(logical->device, swapChain, imageViews, graphicsPipeline);
        commandBuffers = new CommandBuffers(logical->device, logical->family, swapChain, graphicsPipeline, frameBuffers);
        semaphores = new Semaphores(logical->device);
    }

    ////
    // Game Logic
    void drawFrame() {
        uint32_t imageIndex;
        vkAcquireNextImageKHR(
            logical->device,
            swapChain->swapChain,
            std::numeric_limits<uint64_t>::max(),
            semaphores->imageAvailable,
            VK_NULL_HANDLE,
            &imageIndex
        );

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { semaphores->imageAvailable };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers->commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = { semaphores->renderFinished };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(logical->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit queue");

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapchains[] = { swapChain->swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        vkQueuePresentKHR(logical->presentQueue, &presentInfo);
    }

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

            drawFrame();
            SDL_Delay(16);
        }

        vkDeviceWaitIdle(logical->device);
    }

    ////
    // Cleaning Up
    void cleanup() {
        delete semaphores;
        delete commandBuffers;
        delete frameBuffers;
        delete graphicsPipeline;
        delete swapChain;
        delete surface;
        delete logical;
        delete physical;

        if (enableValidationLayer)
            DestroyDebugReportCallbackEXT(instance->instance, callback, nullptr);
        delete instance;

        delete window;
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
