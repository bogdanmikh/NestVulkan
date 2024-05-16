#pragma once

#include <vulkan/vulkan.hpp>
#include "Nest/Renderer/Vulkan/Swapchain.hpp"

using namespace vk;

namespace FrameBufferInit {
    struct FramebufferInput {
        Device device;
        RenderPass renderPass;
        Extent2D swapchainExtent;
    };

    void makeFrameBuffers(const FramebufferInput &inputChunk, std::vector<Swapchain::SwapChainFrame> &frames, bool debug);
}