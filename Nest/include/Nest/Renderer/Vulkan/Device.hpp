#pragma once

#include <vulkan/vulkan.hpp>
#include <array>
#include <optional>

#include "Nest/Logger/Logger.hpp"

using namespace vk;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    inline bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    SurfaceCapabilitiesKHR capabilities;
    std::vector<SurfaceFormatKHR> formats;
    std::vector<PresentModeKHR> presentModes;
};

struct SwapChainBundle {
    SwapchainKHR swapchain;
    std::vector<Image> images;
    Format format;
    Extent2D extent;
};

namespace VulkanInit {
    std::array<bool, 5> getDeviceProperties(const PhysicalDevice &device);

    bool checkDeviceExtensionSupport(const PhysicalDevice &device, const std::vector<const char *> &requestedExtensions,
                                     bool debug);

    bool isSuitable(const PhysicalDevice &device, bool debug);

    QueueFamilyIndices findQueueFamilies(const PhysicalDevice &physicalDevice, const SurfaceKHR& surface, bool debug);

    Device createLogicalDevice(const PhysicalDevice &physicalDevice, const SurfaceKHR &surface, bool debug);

    std::array<Queue, 2> getQueues(const PhysicalDevice &physicalDevice, const Device &device, const SurfaceKHR &surface, bool debug);

    SwapChainSupportDetails querySwapchainSupport(const PhysicalDevice& device, const SurfaceKHR& surface, bool debug);

    SurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<SurfaceFormatKHR> &formats);

    PresentModeKHR chooseSwapchainPresentMode(const std::vector<PresentModeKHR> &presentModes);

    Extent2D chooseSwapchainExtent(uint32_t width, uint32_t height, const SurfaceCapabilitiesKHR &capabilities);

    SwapChainBundle createSwapchain(Device logicalDevice, PhysicalDevice physicalDevice, SurfaceKHR surface, int width, int height, bool debug);
}
