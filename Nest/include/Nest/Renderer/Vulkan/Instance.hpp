#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

using namespace vk;

namespace VulkanInit {
    bool supported(std::vector<const char *> &needExtensions, std::vector<const char *> &layers, bool debug);

    Instance makeInstance(const char *appName, bool debugMode);

    DebugUtilsMessengerEXT makeDebugMessenger(const Instance &instance, const DispatchLoaderDynamic &dld);
}