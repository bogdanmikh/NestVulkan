#pragma once

#include <vulkan/vulkan.hpp>
#include <optional>
#include <cstdint>

using namespace vk;

namespace QueueFamilies {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        inline bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    QueueFamilyIndices findQueueFamilies(const PhysicalDevice &physicalDevice, const SurfaceKHR &surface, bool debug);
};