#include "../vulkan.hpp"

#include "util.hpp"

////
// VkCommandPoolCreateInfo makePoolInfo(uint32_t)
//
// Provides the VkCommandPoolCreateInfo for a pool given its target index.
static VkCommandPoolCreateInfo makePoolInfo(uint32_t queueIndex) {
    VkCommandPoolCreateInfo vertexPoolInfo = {};
    vertexPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vertexPoolInfo.queueFamilyIndex = queueIndex;
    vertexPoolInfo.flags = 0;

    return vertexPoolInfo;
}

namespace wfn_eng::vulkan {
    ////
    // CommandPools
    //
    // Provides a wrapper around the graphics and transfer command pools.

    ////
    // CommandPools(Base&, Device&)
    //
    // Constructs the command pools for graphics and transfer command
    // buffers.
    CommandPools::CommandPools(Base& base, Device& device) {
        util::QueueFamilyIndices queueFamily(
            base.surface(),
            device.physical()
        );

        auto graphicsPoolInfo = makePoolInfo(queueFamily.graphicsFamily);
        auto transferPoolInfo = makePoolInfo(queueFamily.transferFamily);

        if (vkCreateCommandPool(device.logical(), &graphicsPoolInfo, nullptr, &_graphics) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::CommandPools",
                "CommandPools",
                "Failed to create graphics pool"
            );
        }

        if (vkCreateCommandPool(device.logical(), &transferPoolInfo, nullptr, &_transfer) != VK_SUCCESS) {
            throw WfnError(
                "wfn_eng::vulkan::CommandPools",
                "CommandPools",
                "Failed to create transfer pool"
            );
        }
    }

    ////
    // ~CommandPools()
    //
    // Destroys the graphics and transfer command pools.
    CommandPools::~CommandPools() {
        vkDestroyCommandPool(Core::instance().device().logical(), _graphics, nullptr);
        vkDestroyCommandPool(Core::instance().device().logical(), _transfer, nullptr);
    }

    ////
    // VkCommandPool& graphics()
    //
    // Provides reference to the graphics command pool.
    VkCommandPool& CommandPools::graphics() { return _graphics; }

    ////
    // VkCommandPool& transfer()
    //
    // Provides reference to the transfer command pool.
    VkCommandPool& CommandPools::transfer() { return _transfer; }
}
