#pragma once

#include <vulkan/vulkan.hpp>
#include "Swapchain.hpp"

using namespace vk;

namespace Commands {
    struct CommandBufferInputChunk {
        Device device;
        CommandPool commandPool;
        std::vector<Swapchain::SwapChainFrame> &frames;
    };

    CommandPool
    makeCommandPool(const Device &device, const PhysicalDevice &physicalDevice, const SurfaceKHR &surface, bool debug);

    CommandBuffer makeCommandBuffers(const CommandBufferInputChunk &inputChunk, bool debug);
}