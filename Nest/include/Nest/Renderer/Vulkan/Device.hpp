#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <set>
#include <string>
#include <optional>

#include "Nest/Logger/Logger.hpp"

using namespace vk;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

static void logDeviceProperties(const PhysicalDevice &device) {
    /*
        * typedef struct VkPhysicalDeviceProperties {
            uint32_t                            apiVersion;
            uint32_t                            driverVersion;
            uint32_t                            vendorID;
            uint32_t                            deviceID;
            VkPhysicalDeviceType                deviceType;
            char                                deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
            uint8_t                             pipelineCacheUUID[VK_UUID_SIZE];
            VkPhysicalDeviceLimits              limits;
            VkPhysicalDeviceSparseProperties    sparseProperties;
            } VkPhysicalDeviceProperties;
        */


    PhysicalDeviceProperties properties = device.getProperties();

    std::string message = "\n\tDevice name: " + std::string(properties.deviceName) + "\n";
    message += "\tDevice type: ";
    switch (properties.deviceType) {
        case (PhysicalDeviceType::eCpu):
            message += "CPU";
            break;
        case (PhysicalDeviceType::eDiscreteGpu):
            message += "Discrete GPU";
            break;
        case (PhysicalDeviceType::eIntegratedGpu):
            message += "Integrated GPU";
            break;
        case (PhysicalDeviceType::eVirtualGpu):
            message += "Virtual GPU";
            break;
        default:
            message += "Other";
    }
    LOG_INFO("{}", message);
}

static bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device, const std::vector<const char*>& requestedExtensions, bool debug) {
//    Check if a given physical device can satisfy a list of requested device extensions.
    std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());

    std::string message;
    if (debug) {
        message = "Device can support extensions:\n";
    }

    for (vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties()) {
        if (debug) {
            message += "\t\"" + std::string(extension.extensionName) + "\"\n";
        }
        //remove this from the list of required extensions (set checks for equality automatically)
        requiredExtensions.erase(extension.extensionName);
    }

    // if the set is empty then all requirements have been satisfied
    return requiredExtensions.empty();
}

bool isSuitable(const PhysicalDevice &device, bool debug) {
    std::string message;
    message += "Checking if device is suitable";
    const std::vector<const char*> requestedExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    if (debug) {
        message += "\n\tWe are requesting device extensions:";

        for (const char* extension : requestedExtensions) {
            message += "\n\t\"" + std::string(extension) + "\"";
        }
    }

    bool extensionsSupported = checkDeviceExtensionSupport(device, requestedExtensions, debug);
    if (extensionsSupported) {
        if (debug) {
            message += "\n\tDevice can support the requested extensions!";
            LOG_INFO("{}", message);
        }
        return true;
    } else {
        if (debug) {
            message += "\n\tDevice can't support the requested extensions!";
            LOG_INFO("{}", message);
        }
        return false;
    }
}

QueueFamilyIndices& findQueueFamilies(const PhysicalDevice& physicalDevice, bool debug) {
    QueueFamilyIndices indices;

    std::vector<QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

    std::string message;
    if (debug) {
        message += "System can support " + std::to_string(queueFamilies.size()) + " queue families";
    }
    int i = 0;
    for (const auto &queueFamily: queueFamilies) {
        if (queueFamily.queueFlags & QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
            indices.presentFamily = i;
        }
        if (indices.isComplete()) {
            if (debug) {
                message += "\n\tQueue Family " + std::to_string(i) + " is suitable for graphics and presenting";
            }
            break;
        }
        i++;
    }
    if (debug) {
        LOG_INFO("{}", message);
    }
    return indices;
}
