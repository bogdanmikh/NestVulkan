#include <vector>
#include <set>
#include <string>
#include "Nest/Renderer/Vulkan/Device.hpp"
#include "Nest/Renderer/Vulkan/Logging.hpp"
#include "Nest/Renderer/Vulkan/QueueFamilies.hpp"
#include "Nest/Platform/PlatformDetection.hpp"
#include "Nest/Logger/Logger.hpp"

using namespace vk;

std::array<bool, 5> DeviceInit::getDeviceProperties(const PhysicalDevice &device) {
    PhysicalDeviceProperties properties = device.getProperties();

    std::array<bool, 5> typeGPU{false};
    // 0 - Discrete GPU, 1 - Integrated GPU, 2 - Virtual GPU, 3 - CPU, 4 - Other
    switch (properties.deviceType) {
        case (PhysicalDeviceType::eCpu):
            typeGPU[3] = true;
        case (PhysicalDeviceType::eDiscreteGpu):
            typeGPU[0] = true;
        case (PhysicalDeviceType::eIntegratedGpu):
            typeGPU[1] = true;
        case (PhysicalDeviceType::eVirtualGpu):
            typeGPU[2] = true;
        default:
            typeGPU[4] = true;
    }
    return typeGPU;
}

bool DeviceInit::checkDeviceExtensionSupport(const PhysicalDevice &device,
                                             const std::vector<const char *> &requestedExtensions, bool debug) {
//    Check if a given physical device can satisfy a list of requested device extensions.
    std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());

    std::ostringstream message;
    if (debug) {
        message << "Device can support extensions:";
    }
    auto extensions = device.enumerateDeviceExtensionProperties();
    for (const auto &extension: extensions) {
        if (debug) {
            message << "\n\t" << static_cast<const char *>(extension.extensionName);
        }
        //remove this from the list of required extensions (set checks for equality automatically)
        requiredExtensions.erase(extension.extensionName);
    }
    if (debug) {
        LOG_INFO("{}", message.str());
    }
    // if the set is empty then all requirements have been satisfied
    return requiredExtensions.empty();
}

bool DeviceInit::isSuitable(const PhysicalDevice &device, bool debug) {
    std::ostringstream message;
    message << "Checking if device is suitable";
    std::vector<const char *> requestedExtensions;
    requestedExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#ifdef PLATFORM_MACOS
    requestedExtensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
    if (debug) {
        message << "\n\tWe are requesting device extensions:";

        for (const char *extension: requestedExtensions) {
            message << "\n\t\"" << std::string(extension) << "\"";
        }
    }

    bool extensionsSupported = checkDeviceExtensionSupport(device, requestedExtensions, debug);
    if (extensionsSupported) {
        if (debug) {
            message << "\n\tDevice can support the requested extensions!";
            LOG_INFO("{}", message.str());
        }
        return true;
    } else {
        if (debug) {
            message << "\n\tDevice can't support the requested extensions!";
            LOG_INFO("{}", message.str());
        }
        return false;
    }
}


Device DeviceInit::createLogicalDevice(const PhysicalDevice &physicalDevice, const SurfaceKHR &surface, bool debug) {
    QueueFamilies::QueueFamilyIndices indices = QueueFamilies::findQueueFamilies(physicalDevice, surface, debug);
    std::vector<uint32_t> uniqueIndices;
    uniqueIndices.emplace_back(indices.graphicsFamily.value());
    if (indices.graphicsFamily.value() != indices.presentFamily.value()) {
        uniqueIndices.emplace_back(indices.presentFamily.value());
    }

    float queuePriority = 1.0f;

    std::vector<DeviceQueueCreateInfo> queueCreateInfo;
    for (auto &queueFamilyIndex: uniqueIndices) {
        DeviceQueueCreateInfo createInfo;
        createInfo.flags = DeviceQueueCreateFlags();
        createInfo.queueCount = 1;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        createInfo.pQueuePriorities = &queuePriority;
        queueCreateInfo.emplace_back(createInfo);
    }

    PhysicalDeviceFeatures deviceFeatures;

    std::vector<const char *> enabledLayers;
    if (debug) {
        enabledLayers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    std::vector<const char *> extensions;
    extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#ifdef PLATFORM_MACOS
    extensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    DeviceCreateInfo deviceInfo;
    deviceInfo.flags = DeviceCreateFlags();
    deviceInfo.queueCreateInfoCount = queueCreateInfo.size();
    deviceInfo.pQueueCreateInfos = queueCreateInfo.data();
    deviceInfo.enabledLayerCount = enabledLayers.size();
    deviceInfo.ppEnabledLayerNames = enabledLayers.data();
    deviceInfo.enabledExtensionCount = extensions.size();
    deviceInfo.ppEnabledExtensionNames = extensions.data();
    deviceInfo.pEnabledFeatures = &deviceFeatures;

    try {
        Device device = physicalDevice.createDevice(deviceInfo);
        if (debug) {
            LOG_INFO("GPU has been successfully abstracted!");
        }
        return device;
    } catch (const SystemError &err) {
        if (debug) {
            LOG_ERROR("Device creation failed! {}", err.what());
        }
        return nullptr;
    }
}

std::array<Queue, 2>
DeviceInit::getQueues(const PhysicalDevice &physicalDevice, const Device &device, const SurfaceKHR &surface,
                      bool debug) {
    QueueFamilies::QueueFamilyIndices indices = QueueFamilies::findQueueFamilies(physicalDevice, surface, false);

    return {
            device.getQueue(indices.graphicsFamily.value(), 0),
            device.getQueue(indices.presentFamily.value(), 0)
    };
}