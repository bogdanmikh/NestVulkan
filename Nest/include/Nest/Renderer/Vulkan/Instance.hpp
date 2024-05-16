#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

using namespace vk;

namespace InstanceInit {

    bool supported(const std::vector<const char *> &needExtensions, const std::vector<const char *> &layers, bool debug);

    Instance makeInstance(const char *appName, bool debugMode);
};