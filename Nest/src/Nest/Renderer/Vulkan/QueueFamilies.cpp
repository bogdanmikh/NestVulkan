#include "Nest/Renderer/Vulkan/QueueFamilies.hpp"
#include "Nest/Logger/Logger.hpp"
#include "Nest/Settings/SettingsLog.hpp"

QueueFamilyIndices
findQueueFamilies(const PhysicalDevice &physicalDevice, const SurfaceKHR &surface) {
    QueueFamilyIndices indices;
    std::vector<QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

    std::ostringstream message;
    bool debug = VK_PRINT_SUPPORT_QUEUE;
    if (debug) {
        message << "System can support " << queueFamilies.size() << " queue families";
    }
    int i = 0;
    for (const auto &queueFamily: queueFamilies) {
        if (queueFamily.queueFlags & QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
            if (debug) {
                message << "\n\tQueue Family " << i << " is suitable for graphics";
            }
        }

        if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
            indices.presentFamily = i;
            if (debug) {
                message << "\n\tQueue Family " << i << " is suitable for presenting";
            }
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    if (debug) {
        LOG_INFO("{}", message.str());
    }
    return indices;
}