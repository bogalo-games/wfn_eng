#include "../util.hpp"

#include "../../stb_image.h"

namespace wfn_eng::vulkan::util {
    ////
    // class Texture
    //
    // A class built on top of the Buffer and Image structs to provide texture
    // sampling to fragment shaders.

    void Texture::createImage(std::string path) {
        int width, height, channels;
        stbi_uc *pixels = stbi_load(
            path.c_str(),
            &width,
            &height,
            &channels,
            STBI_rgb_alpha
        );

        if (!pixels) {
            throw WfnError(
                "wfn_eng::vulkan::util::Texture",
                "createImage",
                "Failed to load pixels from disk"
            );
        }

        VkDeviceSize size = width * height * channels;
        VkSharingMode sharingMode = Core::instance().device().requiredSharingMode();

        Buffer staging(
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            sharingMode
        );

        staging.copy_from(pixels);
        stbi_image_free(pixels);

        _image = new Image(
            width,
            height,
            channels,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            sharingMode
        );

        _image->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        staging.copy_to(*_image);
        _image->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void Texture::createImageView() {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = _image->handle,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = _image->format,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };

        if (vkCreateImageView(Core::instance().device().logical(), &createInfo, nullptr, &_imageView) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Texture",
                "createImageView",
                "Failed to create image view"
            );
        }
    }

    void Texture::createSampler(VkFilter filter) {
        VkSamplerCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = filter,
            .minFilter = filter,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
            .compareEnable = VK_TRUE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .mipLodBias = 0.0f,
            .minLod = 0.0f,
            .maxLod = 0.0f
        };

        if (vkCreateSampler(Core::instance().device().logical(), &createInfo, nullptr, &_sampler) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::util::Texture",
                "createSampler",
                "Failed to create sampler"
            );
        }
    }

    ////
    // Texture(std::string, VkFilter)
    //
    // Builds a texture from a path on disk, and provides the option of
    // VkFilter for both the mag and min filter.
    Texture::Texture(std::string path, VkFilter filter) {
        createImage(path);
        createImageView();
        createSampler(filter);
    }

    ////
    // Texture(std::string)
    //
    // Builds a texture from a path on disk.
    Texture::Texture(std::string path) :
            Texture(path, VK_FILTER_NEAREST) { }

    ////
    // ~Texture()
    //
    //
    Texture::~Texture() {
        auto& device = Core::instance().device().logical();

        vkDestroySampler(
            device,
            _sampler,
            nullptr
        );

        vkDestroyImageView(
            device,
            _imageView,
            nullptr
        );

        delete _image;
    }

    ////
    // Image& image
    //
    // Provides a reference to the internal image.
    Image& Texture::image() { return *_image; }

    ////
    // VkImageView& imageView()
    //
    // Provides a reference to the VkImageView.
    VkImageView& Texture::imageView() { return _imageView; }

    ////
    // VkSampler& sampler();
    //
    // Provides a reference to the VkSampler.
    VkSampler& Texture::sampler() { return _sampler; }
}
