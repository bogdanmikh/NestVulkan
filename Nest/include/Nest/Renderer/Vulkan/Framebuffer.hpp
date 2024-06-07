#pragma once

#include <vulkan/vulkan.hpp>
#include "Nest/Renderer/Vulkan/Swapchain.hpp"

using namespace vk;

struct FramebufferInput {
    Device device;
    RenderPass renderPass;
    Extent2D swapchainExtent;
};

void makeFrameBuffers(const FramebufferInput &inputChunk, std::vector<SwapChainFrame> &frames, bool debug);