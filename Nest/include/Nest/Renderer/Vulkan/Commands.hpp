#pragma once

#include <vulkan/vulkan.hpp>
#include "Swapchain.hpp"

using namespace vk;

struct CommandBufferInputChunk {
    Device device;
    CommandPool commandPool;
    std::vector<SwapChainFrame> &frames;
};

CommandPool
makeCommandPool(const Device &device, const PhysicalDevice &physicalDevice, const SurfaceKHR &surface, bool debug);

CommandBuffer makeCommandBuffer(const CommandBufferInputChunk &inputChunk);

void makeFrameCommandBuffers(const CommandBufferInputChunk &inputChunk);