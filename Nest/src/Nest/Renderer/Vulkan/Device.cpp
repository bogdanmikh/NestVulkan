#include "Nest/Renderer/Vulkan/Device.hpp"

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    inline bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

void logDeviceProperties(const PhysicalDevice &device) {
    PhysicalDeviceProperties properties = device.getProperties();

    std::string message = "\n\tDevice name: " + std::string(properties.deviceName) + "\n";
    message += "\tDevice type: ";
    switch (properties.deviceType) {
        case (PhysicalDeviceType::eCpu):
            message += "\n\tCPU";
        case (PhysicalDeviceType::eDiscreteGpu):
            message += "\n\tDiscrete GPU";
        case (PhysicalDeviceType::eIntegratedGpu):
            message += "\n\tIntegrated GPU";
        case (PhysicalDeviceType::eVirtualGpu):
            message += "\n\tVirtual GPU";
        default:
            message += "\n\tOther";
    }
    // get size memory for device
    PhysicalDeviceMemoryProperties memoryProperties = device.getMemoryProperties();
    for (int i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        MemoryType memoryType = memoryProperties.memoryTypes[i];
        MemoryPropertyFlags propertyFlag = memoryType.propertyFlags;
        if (propertyFlag & MemoryPropertyFlagBits::eDeviceLocal) {
            DeviceSize heapSize = memoryProperties.memoryHeaps[memoryType.heapIndex].size;
            message += "\n\tHeap size for device local memory: " + std::to_string(heapSize / (1024 * 1024)) + " MB";
        }
    }

    LOG_INFO("{}", message);
}

bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device, const std::vector<const char*>& requestedExtensions, bool debug) {
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

QueueFamilyIndices findQueueFamilies(const PhysicalDevice& physicalDevice, bool debug) {
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
            if (debug) {
                message += "\n\tQueue Family " + std::to_string(i) + " is suitable for graphics and presenting";
            }
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    if (debug) {
        LOG_INFO("{}", message);
    }
    return indices;
}

Device createLogicalDevice(const PhysicalDevice &physicalDevice, bool debug) {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, debug);
    float queuePriority = 1.0f;
    DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.flags = DeviceQueueCreateFlags();
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    PhysicalDeviceFeatures deviceFeatures;

    std::vector<const char*> enabledLayers;
    if (debug) {
        enabledLayers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    DeviceCreateInfo deviceInfo;
    deviceInfo.flags = DeviceCreateFlags();
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceInfo.enabledLayerCount = enabledLayers.size();
    deviceInfo.ppEnabledLayerNames = enabledLayers.data();
    deviceInfo.enabledExtensionCount = 0;
    deviceInfo.ppEnabledExtensionNames = nullptr;
    deviceInfo.pEnabledFeatures = &deviceFeatures;

    try {
        Device device = physicalDevice.createDevice(deviceInfo);
        if (debug) {
            LOG_INFO("GPU has been successfully abstracted!");
        }
        return device;
    } catch (const SystemError& err) {
        if (debug) {
            LOG_ERROR("Device creation failed! {}", err.what());
        }
        return nullptr;
    }
}

Queue getQueue(const PhysicalDevice& physicalDevice, const Device& device, bool debug) {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, debug);

    return device.getQueue(indices.graphicsFamily.value(), 0);
}