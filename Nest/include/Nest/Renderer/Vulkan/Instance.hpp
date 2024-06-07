#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

using namespace vk;

bool supported(const std::vector<const char *> &needExtensions, const std::vector<const char *> &layers);

Instance makeInstanceVulkan(const char *appName);
