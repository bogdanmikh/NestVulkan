#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <set>
#include <string>
#include <optional>

#include "Nest/Logger/Logger.hpp"

using namespace vk;

struct QueueFamilyIndices;

extern void logDeviceProperties(const PhysicalDevice &device);

extern bool
checkDeviceExtensionSupport(const PhysicalDevice &device, const std::vector<const char *> &requestedExtensions,
                            bool debug);

extern bool isSuitable(const PhysicalDevice &device, bool debug);

extern QueueFamilyIndices findQueueFamilies(const PhysicalDevice &physicalDevice, bool debug);

extern Device createLogicalDevice(const PhysicalDevice &physicalDevice, bool debug);

extern Queue getQueue(const PhysicalDevice &physicalDevice, const Device &device, bool debug);