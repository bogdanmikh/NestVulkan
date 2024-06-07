#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

using namespace vk;

struct SwapChainSupportDetails {
    SurfaceCapabilitiesKHR capabilities;
    std::vector<SurfaceFormatKHR> formats;
    std::vector<PresentModeKHR> presentModes;
};

struct SwapChainFrame {
    Image image;
    ImageView imageView;
    Framebuffer framebuffer;
    CommandBuffer commandBuffer;
    Semaphore imageAvailable, renderFinished;
    Fence inFlight;
};

struct SwapChainBundle {
    SwapchainKHR swapchain;
    std::vector<SwapChainFrame> frames;
    Format format;
    Extent2D extent;
};

SwapChainSupportDetails querySwapchainSupport(const PhysicalDevice &device, const SurfaceKHR &surface);

SurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<SurfaceFormatKHR> &formats);

PresentModeKHR chooseSwapchainPresentMode(const std::vector<PresentModeKHR> &presentModes);

Extent2D chooseSwapchainExtent(uint32_t width, uint32_t height, const SurfaceCapabilitiesKHR &capabilities);

SwapChainBundle
createSwapchain(Device logicalDevice, PhysicalDevice physicalDevice, SurfaceKHR surface, int width, int height, SwapchainKHR *oldSwapchain = nullptr);



