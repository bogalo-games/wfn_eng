#ifndef __WFN_ENG_VULKAN_HPP__
#define __WFN_ENG_VULKAN_HPP__

#include <vulkan/vulkan.h>
#include <vector>

#include "error.hpp"
#include "sdl.hpp"

namespace wfn_eng::vulkan {
    class Base;
    class Device;
    class Swapchain;
    class Core;

    ////
    // namespace util
    //
    // A collection of utility classes that are not directly used in rendering,
    // but are used in the construction of the graphics pipeline.
    namespace util {
        ////
        // struct QueueFamilyIndices
        //
        // Utility struct that contains information on the indices of various
        // queue families (aka the IDs of VkQueues).
        struct QueueFamilyIndices {
            ////
            // int graphicsFamily
            //
            // The index of the graphics queue.
            int graphicsFamily = -1;

            ////
            // int presentationFamily
            //
            // The index of the presentation queue.
            int presentationFamily = -1;

            ////
            // QueueFamilyIndices(VkSurfaceKHR, VkPhysicalDevice)
            //
            // Given the reference to a VkSurfaceKHR and a VkPhysicalDevice,
            // query the relevant queue indices.
            QueueFamilyIndices(VkSurfaceKHR, VkPhysicalDevice);

            ////
            // QueueFamilyIndices(Base&, Device&)
            //
            // Given the reference to a Base and a Device, query the relevant
            // queue indices.
            QueueFamilyIndices(Base&, Device&);

            ////
            // bool sufficient
            //
            // Checks if the queue family's indices are sufficient for use in
            // the rest of the program.
            bool sufficient();
        };

        ////
        // struct SwapchainSupport
        //
        // Utility struct to encapsulate various swapchain capabilities of a
        // VkPhysicalDevice.
        struct SwapchainSupport {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;

            ////
            // SwapchainSupport(VkSurfaceKHR, VkPhysicalDevice)
            //
            // Queries the swapchain capabilities of a VkSurfaceKHR and a
            // VkPhysicalDevice directly.
            SwapchainSupport(VkSurfaceKHR, VkPhysicalDevice);

            ////
            // SwapchainSupport(Base&, Device&)
            //
            // Queries the swapchain capabilities of a VkPhysicalDevice via a
            // reference to the Base and Device classes (below) that encapsulate
            // the surface and physical device.
            SwapchainSupport(Base&, Device&);

            ////
            // bool sufficient
            //
            // Returns whether the queried swapchain capabilities are sufficient
            // to use.
            bool sufficient();
        };
    }

    ////
    // class Base
    //
    // A container for the base-level features of Vulkan, aka the VkInstance and
    // the VkSurfaceKHR.
    class Base {
        VkInstance _instance;
        VkSurfaceKHR _surface;

    public:
        inline static const std::vector<const char *> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        ////
        // Base(sdl::Window&)
        //
        // Construct the base of the Vulkan instance (VkInstance and
        // VkSurfaceKHR) from an SDL window wrapper.
        Base(sdl::Window&);

        ////
        // ~Base()
        //
        // Destroying the VkInstance and VkSurfaceKHR.
        ~Base();

        ////
        // VkInstance instance()
        //
        // Provides access to the VkInstance.
        VkInstance& instance();

        ////
        // VkSurfaceKHR surface()
        //
        // Provides access to the VkSurfaceKHR.
        VkSurfaceKHR& surface();


        // Following Rule of 3's
        Base(const Base&) = delete;
        Base& operator=(const Base&) = delete;
    };

    ////
    // class Device
    //
    // A container for the device-related features of Vulkan, that includes the
    // physical and logical devices, along with their relevant queues.
    class Device {
        VkPhysicalDevice _physical;
        VkDevice _logical;
        VkQueue _graphicsQueue;
        VkQueue _presentationQueue;

        ////
        // makePhysicalDevice
        //
        // Constructs the VkPhysicalDevice.
        void makePhysicalDevice(Base&);

        ////
        // makeLogicalDevice
        //
        // Constructs the VkDevice, along with both the graphics and
        // presentation queues.
        void makeLogicalDevice(Base&);

    public:
        ////
        // Device(Base&)
        //
        // Constructing a device from a Base.
        Device(Base&);

        ////
        // ~Device()
        //
        // Destroying the Device.
        ~Device();

        ////
        // VkPhysicalDevice physical()
        //
        // Getting the reference to the VkPhysicalDevice.
        VkPhysicalDevice& physical();

        ////
        // VkDevice logical()
        //
        // Getting the reference to the VkDevice.
        VkDevice& logical();

        ////
        // VkQueue graphicsQueue()
        //
        // Getting the graphics queue.
        VkQueue& graphicsQueue();

        ////
        // VkQueue presentationQueue()
        //
        // Getting the presentation queue.
        VkQueue& presentationQueue();


        // Following Rule of 3's
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
    };

    ////
    // Swapchain
    //
    // TODO: Documentation
    class Swapchain {
        VkSwapchainKHR _swapchain;
        std::vector<VkImage> _images;
        VkFormat _format;
        VkExtent2D _extent;
        std::vector<VkImageView> _imageViews;
        std::vector<VkFramebuffer> _frameBuffers;

    public:
        Swapchain();
        ~Swapchain();

        VkSwapchainKHR& get();
        std::vector<VkImage>& images();
        VkFormat& format();
        VkExtent2D& extent();
        std::vector<VkImageView&> imageViews();
        std::vector<VkFramebuffer&> frameBuffers();

        Swapchain(const Swapchain&) = delete;
        Swapchain& operator=(const Swapchain&) = delete;
    };

    ////
    // Core
    //
    // TODO: Documentation
    class Core {
        Base base;
        Device device;
        Swapchain swapchain;

    public:
        Core(wfn_eng::sdl::Window&);
        ~Core();

        Core(const Core&) = delete;
        Core& operator=(const Core&) = delete;
    };
}

#endif
