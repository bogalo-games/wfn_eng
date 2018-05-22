#include "../vulkan.hpp"

namespace wfn_eng::vulkan {
    ////
    // Swapchain
    //
    // A container for the VkSwapchainKHR and all associated data, including
    // the swapchain's VkImages, the format, the extent, and the VkImageViews,
    // and the VkFramebuffers.

    ////
    // void makeSwapchain()
    //
    // Constructs the VkSwapchainKHR and the swapchain's VkImages.
    void Swapchain::makeSwapchain() {

    }

    ////
    // void makeImageViews()
    //
    // Constructs the swapchain's VkImageViews.
    void Swapchain::makeImageViews() {

    }

    ////
    // void makeFrameBuffers()
    //
    // Constructs the swapchain's VkFramebuffers.
    void Swapchain::makeFrameBuffers() {

    }

    ////
    // Swapchain
    //
    // Constructing a swapchain, along with all its attachments (e.g.
    // swapchain images and frame buffers).
    Swapchain::Swapchain(Base& base, Device& device) {

    }

    ////
    // ~Swapchain
    //
    // Cleaning up the Swapchain. Note that it requires a reference to the
    // VkDevice, which is included in this class for that purpose.
    Swapchain::~Swapchain() {

    }

    ////
    // VkSwapchainKHR& get()
    //
    // Provides a reference to the swapchain.
    VkSwapchainKHR& Swapchain::get() { return _swapchain; }

    ////
    // std::vector<VkImage>& images()
    //
    // Provides a reference to the swapchain's images.
    std::vector<VkImage>& Swapchain::images() { return _images; }

    ////
    // VkFormat& format()
    //
    // Provides a reference to the images' format.
    VkFormat& Swapchain::format() { return _format; }

    ////
    // VkExtent2D& extent()
    //
    // Provides a reference to the images' extent.
    VkExtent2D& Swapchain::extent() { return _extent; }

    ////
    // std::vector<VkImageView>& imageViews()
    //
    // Provides a reference to the image views.
    std::vector<VkImageView>& Swapchain::imageViews() { return _imageViews; }

    ////
    // std::vector<VkFramebuffer>& frameBuffers();
    //
    // Provides a reference to the framebuffers.
    std::vector<VkFramebuffer>& Swapchain::frameBuffers() { return _frameBuffers; }

    // void createSwapChain() {
    //     wfn_eng::vulkan::util::SwapchainSupport swapChainSupport(base->surface(), device->physical());
    //
    //     VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    //     VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    //     VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
    //
    //     uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    //     if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    //         imageCount = swapChainSupport.capabilities.maxImageCount;
    //     }
    //
    //     VkSwapchainCreateInfoKHR createInfo = {};
    //     createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    //     createInfo.surface = base->surface();
    //
    //     createInfo.minImageCount = imageCount;
    //     createInfo.imageFormat = surfaceFormat.format;
    //     createInfo.imageColorSpace = surfaceFormat.colorSpace;
    //     createInfo.imageExtent = extent;
    //     createInfo.imageArrayLayers = 1;
    //     createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    //
    //     wfn_eng::vulkan::util::QueueFamilyIndices indices(base->surface(), device->physical());
    //     uint32_t queueFamilyIndices[] = {(uint32_t) indices.graphicsFamily, (uint32_t) indices.presentationFamily};
    //
    //     if (indices.graphicsFamily != indices.presentationFamily) {
    //         createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    //         createInfo.queueFamilyIndexCount = 2;
    //         createInfo.pQueueFamilyIndices = queueFamilyIndices;
    //     } else {
    //         createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //     }
    //
    //     createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    //     createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    //     createInfo.presentMode = presentMode;
    //     createInfo.clipped = VK_TRUE;
    //
    //     createInfo.oldSwapchain = VK_NULL_HANDLE;
    //
    //     if (vkCreateSwapchainKHR(device->logical(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
    //         throw std::runtime_error("failed to create swap chain!");
    //     }
    //
    //     vkGetSwapchainImagesKHR(device->logical(), swapChain, &imageCount, nullptr);
    //     swapChainImages.resize(imageCount);
    //     vkGetSwapchainImagesKHR(device->logical(), swapChain, &imageCount, swapChainImages.data());
    //
    //     swapChainImageFormat = surfaceFormat.format;
    //     swapChainExtent = extent;
    // }
    //
    // void createImageViews() {
    //     swapChainImageViews.resize(swapChainImages.size());
    //
    //     for (size_t i = 0; i < swapChainImages.size(); i++) {
    //         VkImageViewCreateInfo createInfo = {};
    //         createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    //         createInfo.image = swapChainImages[i];
    //         createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    //         createInfo.format = swapChainImageFormat;
    //         createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    //         createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    //         createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    //         createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    //         createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //         createInfo.subresourceRange.baseMipLevel = 0;
    //         createInfo.subresourceRange.levelCount = 1;
    //         createInfo.subresourceRange.baseArrayLayer = 0;
    //         createInfo.subresourceRange.layerCount = 1;
    //
    //         if (vkCreateImageView(device->logical(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
    //             throw std::runtime_error("failed to create image views!");
    //         }
    //     }
    // }
    //
    // void createFramebuffers() {
    //     swapChainFramebuffers.resize(swapChainImageViews.size());
    //
    //     for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    //         VkImageView attachments[] = {
    //             swapChainImageViews[i]
    //         };
    //
    //         VkFramebufferCreateInfo framebufferInfo = {};
    //         framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    //         framebufferInfo.renderPass = renderPass;
    //         framebufferInfo.attachmentCount = 1;
    //         framebufferInfo.pAttachments = attachments;
    //         framebufferInfo.width = swapChainExtent.width;
    //         framebufferInfo.height = swapChainExtent.height;
    //         framebufferInfo.layers = 1;
    //
    //         if (vkCreateFramebuffer(device->logical(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
    //             throw std::runtime_error("failed to create framebuffer!");
    //         }
    //     }
    // }
}
