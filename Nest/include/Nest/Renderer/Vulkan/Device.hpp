#pragma once

#include <vulkan/vulkan.hpp>
#include <array>
#include <optional>

using namespace vk;

class DeviceInit {
public:
    static std::array<bool, 5> getDeviceProperties(const PhysicalDevice &device);

    static bool
    checkDeviceExtensionSupport(const PhysicalDevice &device, const std::vector<const char *> &requestedExtensions,
                                bool debug);

    static bool isSuitable(const PhysicalDevice &device, bool debug);

    static Device createLogicalDevice(const PhysicalDevice &physicalDevice, const SurfaceKHR &surface, bool debug);

    static std::array<Queue, 2>
    getQueues(const PhysicalDevice &physicalDevice, const Device &device, const SurfaceKHR &surface, bool debug);
};
