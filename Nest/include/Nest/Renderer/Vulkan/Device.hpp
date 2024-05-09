#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <set>
#include <string>
#include <optional>
#include <array>

#include "Nest/Logger/Logger.hpp"

using namespace vk;

struct QueueFamilyIndices;

namespace DeviceInit {
    void logDeviceProperties(const PhysicalDevice &device);

    bool checkDeviceExtensionSupport(const PhysicalDevice &device, const std::vector<const char *> &requestedExtensions,
                                bool debug);

    bool isSuitable(const PhysicalDevice &device, bool debug);

    QueueFamilyIndices findQueueFamilies(const PhysicalDevice &physicalDevice, const SurfaceKHR& surface, bool debug);

    Device createLogicalDevice(const PhysicalDevice &physicalDevice, const SurfaceKHR &surface, bool debug);

    std::array<Queue, 2> getQueues(const PhysicalDevice &physicalDevice, const Device &device, const SurfaceKHR &surface, bool debug);
}