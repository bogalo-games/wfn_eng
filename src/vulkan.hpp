#ifndef __WFN_ENG_VULKAN_HPP__
#define __WFN_ENG_VULKAN_HPP__

#include <vulkan/vulkan.h>
#include <vector>

#include "sdl.hpp"

namespace wfn_eng::vulkan {
    ////
    // namespace util
    //
    // A collection of utility classes that are not directly used in rendering,
    // but are used in the construction of the graphics pipeline.
    namespace util {
        ////
        // struct QueueFamily
        //
        //
        struct QueueFamily {

        };

        ////
        // struct SwapchainSupport
        //
        //
        struct SwapchainSupport {

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
        Base();
        ~Base();

        VkInstance& instance();
        VkSurfaceKHR& surface();


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

    public:
        Device();
        ~Device();

        VkPhysicalDevice& physical();
        VkDevice& logical();

        VkQueue& graphicsQueue();
        VkQueue& presentationQueue;


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
