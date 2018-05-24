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


        // Following Rule of 5's
        Base(const Base&) = delete;
        Base(Base&&) = delete;
        Base& operator=(const Base&) = delete;
        Base& operator=(Base&&) = delete;
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


        // Following Rule of 5's
        Device(const Device&) = delete;
        Device(Device&&) = delete;
        Device& operator=(const Device&) = delete;
        Device& operator=(Device&&) = delete;
    };

    ////
    // Swapchain
    //
    // A container for the VkSwapchainKHR and all associated data, including
    // the swapchain's VkImages, the format, the extent, and the VkImageViews,
    // and the VkFramebuffers.
    class Swapchain {
        VkSwapchainKHR _swapchain;
        std::vector<VkImage> _images;
        VkFormat _format;
        VkExtent2D _extent;
        std::vector<VkImageView> _imageViews;
        VkRenderPass _renderPass;
        std::vector<VkFramebuffer> _frameBuffers;

        VkDevice device;

        ////
        // void makeSwapchain(Base&, Device&)
        //
        // Constructs the VkSwapchainKHR and the swapchain's VkImages.
        void makeSwapchain(Base&, Device&);

        ////
        // void makeImageViews(Device&)
        //
        // Constructs the swapchain's VkImageViews.
        void makeImageViews(Device&);

        ////
        // void makeRenderPass(Device&)
        //
        // Constructs the swapchain's VkRenderPass.
        void makeRenderPass(Device&);

        ////
        // void makeFrameBuffers(Device&)
        //
        // Constructs the swapchain's VkFramebuffers.
        void makeFrameBuffers(Device&);

    public:
        ////
        // Swapchain
        //
        // Constructing a swapchain, along with all its attachments (e.g.
        // swapchain images and frame buffers).
        Swapchain(Base&, Device&);

        ////
        // ~Swapchain
        //
        // Cleaning up the Swapchain. Note that it requires a reference to the
        // VkDevice, which is included in this class for that purpose.
        ~Swapchain();

        ////
        // VkSwapchainKHR& get()
        //
        // Provides a reference to the swapchain.
        VkSwapchainKHR& get();

        ////
        // std::vector<VkImage>& images()
        //
        // Provides a reference to the swapchain's images.
        std::vector<VkImage>& images();

        ////
        // VkFormat& format()
        //
        // Provides a reference to the images' format.
        VkFormat& format();

        ////
        // VkExtent2D& extent()
        //
        // Provides a reference to the images' extent.
        VkExtent2D& extent();

        ////
        // std::vector<VkImageView>& imageViews()
        //
        // Provides a reference to the image views.
        std::vector<VkImageView>& imageViews();

        ////
        // VkRenderPass& renderPass()
        //
        // Provides a reference to the render pass.
        VkRenderPass& renderPass();

        ////
        // std::vector<VkFramebuffer>& frameBuffers();
        //
        // Provides a reference to the framebuffers.
        std::vector<VkFramebuffer>& frameBuffers();


        // Following Rule of 5's
        Swapchain(const Swapchain&) = delete;
        Swapchain(Swapchain&&) = delete;
        Swapchain& operator=(const Swapchain&) = delete;
        Swapchain& operator=(Swapchain&&) = delete;
    };

    ////
    // Core
    //
    // A wrapper around the Base, Device, and Swapchain implementations to
    // provide a central wrapper around the core functionality of Vulkan.
    //
    // Does not include reference to graphics pipelines or command buffers
    // because they are application specific.
    class Core {
        Base *_base;
        Device *_device;
        Swapchain *_swapchain;

    public:
        ////
        // Core(wnf_eng::sdl::Window&)
        //
        // Constructs the Base, Device, and Swapchain, given reference to an SDL
        // window wrapper.
        Core(wfn_eng::sdl::Window&);

        ////
        // ~Core()
        //
        // Handles the destruction of the Base, Device, and Swapchain.
        ~Core();

        ////
        // Base& base()
        //
        // Returns the Base reference.
        Base& base();

        ////
        // Device& device()
        //
        // Returns the Device reference.
        Device& device();

        ////
        // Swapchain& swapchain()
        //
        // Returns the Swapchain reference.
        Swapchain& swapchain();


        // Following Rule of 5's
        Core(const Core&) = delete;
        Core(Core&&) = delete;
        Core& operator=(const Core&) = delete;
        Core& operator=(Core&&) = delete;
    };
}

#endif
