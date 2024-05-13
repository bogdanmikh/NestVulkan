#pragma once

#include <vulkan/vulkan.hpp>
#include <array>
#include <optional>

using namespace vk;

namespace VulkanInit {
    std::array<bool, 5> getDeviceProperties(const PhysicalDevice &device);

    bool checkDeviceExtensionSupport(const PhysicalDevice &device, const std::vector<const char *> &requestedExtensions,
                                     bool debug);

    bool isSuitable(const PhysicalDevice &device, bool debug);

    Device createLogicalDevice(const PhysicalDevice &physicalDevice, const SurfaceKHR &surface, bool debug);

    std::array<Queue, 2>
    getQueues(const PhysicalDevice &physicalDevice, const Device &device, const SurfaceKHR &surface, bool debug);

}
