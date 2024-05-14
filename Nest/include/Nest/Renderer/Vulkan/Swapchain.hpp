#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

using namespace vk;

class Swapchain {
public:
    struct SwapChainSupportDetails {
        SurfaceCapabilitiesKHR capabilities;
        std::vector<SurfaceFormatKHR> formats;
        std::vector<PresentModeKHR> presentModes;
    };

    struct SwapChainFrame {
        Image image;
        ImageView imageView;
    };

    struct SwapChainBundle {
        SwapchainKHR swapchain;
        std::vector<SwapChainFrame> frames;
        Format format;
        Extent2D extent;
    };

    static SwapChainSupportDetails querySwapchainSupport(const PhysicalDevice& device, const SurfaceKHR& surface, bool debug);

    static SurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<SurfaceFormatKHR> &formats);

    static PresentModeKHR chooseSwapchainPresentMode(const std::vector<PresentModeKHR> &presentModes);

    static Extent2D chooseSwapchainExtent(uint32_t width, uint32_t height, const SurfaceCapabilitiesKHR &capabilities);

    static SwapChainBundle createSwapchain(Device logicalDevice, PhysicalDevice physicalDevice, SurfaceKHR surface, int width, int height, bool debug);
};

