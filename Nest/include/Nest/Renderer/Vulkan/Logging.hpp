#pragma once

#include <vulkan/vulkan.hpp>

using namespace vk;

namespace VulkanLogging {
    DebugUtilsMessengerEXT makeDebugMessenger(const Instance &instance, const DispatchLoaderDynamic &dld);

    void logDeviceProperties(const PhysicalDevice &device, const std::array<bool, 5> &types);

    std::vector<const char *> logTransformBits(const SurfaceTransformFlagsKHR &bits);

    std::vector<const char *> logAlphaCompositeBits(const CompositeAlphaFlagsKHR &bits);

    std::vector<const char *> logImageUsageBits(const ImageUsageFlags &bits);

    const char *logPresentMode(const PresentModeKHR &presentMode);
};
