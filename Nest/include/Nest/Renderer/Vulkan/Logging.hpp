#pragma once

#include <vulkan/vulkan.hpp>

using namespace vk;

class VulkanLogging {
public:
    static DebugUtilsMessengerEXT makeDebugMessenger(const Instance &instance, const DispatchLoaderDynamic &dld);

    static void logDeviceProperties(const PhysicalDevice &device, const std::array<bool, 5> &types);

    static std::vector<const char *> logTransformBits(const SurfaceTransformFlagsKHR &bits);

    static std::vector<const char *> logAlphaCompositeBits(const CompositeAlphaFlagsKHR &bits);

    static std::vector<const char *> logImageUsageBits(const ImageUsageFlags &bits);

    static const char* logPresentMode(const PresentModeKHR& presentMode);
};
