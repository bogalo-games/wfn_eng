#include "../vulkan.hpp"

#include <iostream>

#include "util.hpp"

////
// VkSurfaceFormatKHR chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR>&)
//
// Given a series of available formats, choose one!
static VkSurfaceFormatKHR chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR>& formats) {
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        return {
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        };
    }

    for (const auto& format: formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    return formats[0];
}

////
// VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>&)
//
// Given a series of available presentation modes, select a presentation mode.
static VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& presentMode: presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return presentMode;
        else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            bestMode = presentMode;
    }

    return bestMode;
}

////
// VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR&)
//
// Given the capabilities of the surface, select an extent.
static VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {640, 480};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

namespace wfn_eng::vulkan {
    ////
    // Swapchain
    //
    // A container for the VkSwapchainKHR and all associated data, including
    // the swapchain's VkImages, the format, the extent, and the VkImageViews,
    // and the VkFramebuffers.

    ////
    // void makeSwapchain(Base&, Device&)
    //
    // Constructs the VkSwapchainKHR and the swapchain's VkImages.
    void Swapchain::makeSwapchain(Base& base, Device& device) {
        wfn_eng::vulkan::util::SwapchainSupport swapchainSupport(base.surface(), device.physical());

        VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapchainSupport.formats);
        VkPresentModeKHR presentMode = choosePresentMode(swapchainSupport.presentModes);
        VkExtent2D extent = chooseExtent(swapchainSupport.capabilities);

        uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
        if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
            imageCount = swapchainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = base.surface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        wfn_eng::vulkan::util::QueueFamilyIndices indices(base.surface(), device.physical());
        uint32_t queueFamilyIndices[] = {
            (uint32_t)indices.graphicsFamily,
            (uint32_t)indices.presentationFamily
        };

        if (indices.graphicsFamily != indices.presentationFamily) {
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }

        createInfo.imageSharingMode = device.requiredSharingMode();
        createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device.logical(), &createInfo, nullptr, &_swapchain) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::Swapchain",
                "makeSwapchain",
                "Create Swapchain"
            );
        }

        vkGetSwapchainImagesKHR(device.logical(), _swapchain, &imageCount, nullptr);
        _images.resize(imageCount);
        vkGetSwapchainImagesKHR(device.logical(), _swapchain, &imageCount, _images.data());

        _format = surfaceFormat.format;
        _extent = extent;
    }

    ////
    // void makeImageViews(Device&)
    //
    // Constructs the swapchain's VkImageViews.
    void Swapchain::makeImageViews(Device& device) {
        _imageViews.resize(_images.size());
        for (size_t i = 0; i < _images.size(); i++) {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = _images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = _format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device.logical(), &createInfo, nullptr, &_imageViews[i]) != VK_SUCCESS) {
                throw WfnError(
                    "wfn_eng::vulkan::Swapchain",
                    "makeImageViews",
                    "Create Image View"
                );
            }
        }
    }

    ////
    // void makeRenderPass(Device&)
    //
    // Constructs the swapchain's VkRenderPass.
    void Swapchain::makeRenderPass(Device& device) {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = _format;
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

        if (vkCreateRenderPass(device.logical(), &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::Swapchain",
                "makeRenderPass",
                "Create Render Pass"
            );
        }
    }

    ////
    // void makeFrameBuffers(Device&)
    //
    // Constructs the swapchain's VkFramebuffers.
    void Swapchain::makeFrameBuffers(Device& device) {
        _frameBuffers.resize(_imageViews.size());
        for (size_t i = 0; i < _imageViews.size(); i++) {
            VkImageView attachments[] = {
                _imageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = _renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = _extent.width;
            framebufferInfo.height = _extent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device.logical(), &framebufferInfo, nullptr, &_frameBuffers[i]) != VK_SUCCESS) {
                throw WfnError(
                    "wfn_eng::vulkan::Swapchain",
                    "makeFrameBuffers",
                    "Create Frame Buffer"
                );
            }
        }
    }

    ////
    // Swapchain
    //
    // Constructing a swapchain, along with all its attachments (e.g.
    // swapchain images and frame buffers).
    Swapchain::Swapchain(Base& base, Device& device) {
        makeSwapchain(base, device);
        makeImageViews(device);
        makeRenderPass(device);
        makeFrameBuffers(device);

        this->device = device.logical();
    }

    ////
    // ~Swapchain
    //
    // Cleaning up the Swapchain. Note that it requires a reference to the
    // VkDevice, which is included in this class for that purpose.
    Swapchain::~Swapchain() {
        vkDestroyRenderPass(device, _renderPass, nullptr);
        for (auto imageView: _imageViews)
            vkDestroyImageView(device, imageView, nullptr);
        vkDestroySwapchainKHR(device, _swapchain, nullptr);
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
    // VkRenderPass& renderPass()
    //
    // Provides a reference to the render pass.
    VkRenderPass& Swapchain::renderPass() { return _renderPass; };

    ////
    // std::vector<VkFramebuffer>& frameBuffers();
    //
    // Provides a reference to the framebuffers.
    std::vector<VkFramebuffer>& Swapchain::frameBuffers() { return _frameBuffers; }
}
