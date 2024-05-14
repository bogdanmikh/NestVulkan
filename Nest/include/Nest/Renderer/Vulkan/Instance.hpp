#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

using namespace vk;

class InstanceInit {
public:
    static bool supported(std::vector<const char *> &needExtensions, std::vector<const char *> &layers, bool debug);

    static Instance makeInstance(const char *appName, bool debugMode);
};