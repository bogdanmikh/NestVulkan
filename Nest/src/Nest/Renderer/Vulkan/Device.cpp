#include <vector>
#include <set>
#include <string>
#include "Nest/Renderer/Vulkan/Device.hpp"
#include "Nest/Renderer/Vulkan/Logging.hpp"
#include "Nest/Platform/PlatformDetection.hpp"

using namespace vk;

std::array<bool, 5> VulkanInit::getDeviceProperties(const PhysicalDevice &device) {
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

bool VulkanInit::checkDeviceExtensionSupport(const PhysicalDevice &device,
                                             const std::vector<const char *> &requestedExtensions, bool debug) {
//    Check if a given physical device can satisfy a list of requested device extensions.
    std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());

    std::ostringstream stringStream;
    if (debug) {
        stringStream << "Device can support extensions:";
    }
    auto extensions = device.enumerateDeviceExtensionProperties();
    for (const auto &extension: extensions) {
        if (debug) {
            stringStream << "\n\t" << static_cast<const char *>(extension.extensionName);
        }
        //remove this from the list of required extensions (set checks for equality automatically)
        requiredExtensions.erase(extension.extensionName);
    }
    if (debug) {
        LOG_INFO("{}", stringStream.str());
    }
    // if the set is empty then all requirements have been satisfied
    return requiredExtensions.empty();
}

bool VulkanInit::isSuitable(const PhysicalDevice &device, bool debug) {
    std::ostringstream stringStream;
    stringStream << "Checking if device is suitable";
    const std::vector<const char *> requestedExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
#ifdef PLATFORM_MACOS
    requestedExtensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
    if (debug) {
        stringStream << "\n\tWe are requesting device extensions:";

        for (const char *extension: requestedExtensions) {
            stringStream << "\n\t\"" << std::string(extension) << "\"";
        }
    }

    bool extensionsSupported = checkDeviceExtensionSupport(device, requestedExtensions, debug);
    if (extensionsSupported) {
        if (debug) {
            stringStream << "\n\tDevice can support the requested extensions!";
            LOG_INFO("{}", stringStream.str());
        }
        return true;
    } else {
        if (debug) {
            stringStream << "\n\tDevice can't support the requested extensions!";
            LOG_INFO("{}", stringStream.str());
        }
        return false;
    }
}

QueueFamilyIndices
VulkanInit::findQueueFamilies(const PhysicalDevice &physicalDevice, const SurfaceKHR &surface, bool debug) {
    QueueFamilyIndices indices;
    std::vector<QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

    std::ostringstream stringStream;
    if (debug) {
        stringStream << "System can support " << queueFamilies.size() << " queue families";
    }
    int i = 0;
    for (const auto &queueFamily: queueFamilies) {
        if (queueFamily.queueFlags & QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
            if (debug) {
                stringStream << "\n\tQueue Family " << i << " is suitable for graphics";
            }
        }

        if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
            indices.presentFamily = i;
            if (debug) {
                stringStream << "\n\tQueue Family " << i << " is suitable for presenting";
            }
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    if (debug) {
        LOG_INFO("{}", stringStream.str());
    }
    return indices;
}

Device VulkanInit::createLogicalDevice(const PhysicalDevice &physicalDevice, const SurfaceKHR &surface, bool debug) {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface, debug);
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
VulkanInit::getQueues(const PhysicalDevice &physicalDevice, const Device &device, const SurfaceKHR &surface,
                      bool debug) {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface, false);

    return {
            device.getQueue(indices.graphicsFamily.value(), 0),
            device.getQueue(indices.presentFamily.value(), 0)
    };
}

SwapChainSupportDetails
VulkanInit::querySwapchainSupport(const PhysicalDevice &device, const SurfaceKHR &surface, bool debug) {
    SwapChainSupportDetails support;
    support.capabilities = device.getSurfaceCapabilitiesKHR(surface);

    std::ostringstream stringStream;
    if (debug) {
        stringStream << "\n\tMinimum image count:" << support.capabilities.minImageCount << "\n";
        stringStream << "\tMaximum image count:" << support.capabilities.maxImageCount << "\n";

        stringStream << "\t\tWidth: " << support.capabilities.currentExtent.width << "\n";
        stringStream << "\t\tHeight: " << support.capabilities.currentExtent.height << "\n";

        stringStream << "\tMinimum supported extent: \n";
        stringStream << "\t\tWidth: " << support.capabilities.minImageExtent.width << "\n";
        stringStream << "\t\tHeight: " << support.capabilities.minImageExtent.height << "\n";

        stringStream << "\tMaximum supported extent: \n";
        stringStream << "\t\tWidth: " << support.capabilities.maxImageExtent.width << "\n";
        stringStream << "\t\tHeight: " << support.capabilities.maxImageExtent.height << "\n";

        stringStream << "\tMaximum image array layers: " << support.capabilities.maxImageArrayLayers << "\n";

        stringStream << "\tSupported transforms:\n";
        std::vector<const char *> stringList = VulkanLogging::logTransformBits(
                support.capabilities.supportedTransforms);
        for (const auto &line: stringList) {
            stringStream << "\t\t" << line << '\n';
        }

        stringStream << "\tCurrent transform:\n";
        stringList = VulkanLogging::logTransformBits(support.capabilities.currentTransform);
        for (const auto &line: stringList) {
            stringStream << "\t\t" << line << '\n';
        }

        stringStream << "\tSupported alpha operations:\n";
        stringList = VulkanLogging::logAlphaCompositeBits(support.capabilities.supportedCompositeAlpha);
        for (const auto &line: stringList) {
            stringStream << "\t\t" << line << "\n";
        }

        stringStream << "\tsupported image usage:\n";
        stringList = VulkanLogging::logImageUsageBits(support.capabilities.supportedUsageFlags);
        for (const auto &line: stringList) {
            stringStream << "\t\t" << line << "\n";
        }
    }
    support.formats = device.getSurfaceFormatsKHR(surface);

    if (debug) {
        for (const auto &supportedFormat: support.formats) {
            stringStream << "\tSupported pixel format: " << to_string(supportedFormat.format) << "\n";
            stringStream << "\tSupported color space: " << to_string(supportedFormat.colorSpace) << "\n";
        }
    }
    support.presentModes = device.getSurfacePresentModesKHR(surface);

    if (debug) {
        for (const auto &presentMode: support.presentModes) {
            stringStream << "\t" << VulkanLogging::logPresentMode(presentMode) << "\n";
        }
        std::string message = std::string(stringStream.str());
        message.erase(message.end() - 1);
        LOG_INFO("{}", message.c_str());
    }
    return support;
}

SurfaceFormatKHR VulkanInit::chooseSwapchainSurfaceFormat(const std::vector<SurfaceFormatKHR> &formats) {
    for (const auto &format: formats) {
        // color format an 0-255, non linear
        if (format.format == Format::eB8G8R8A8Unorm
            && format.colorSpace == ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }
    return formats[0];
}

PresentModeKHR VulkanInit::chooseSwapchainPresentMode(const std::vector<PresentModeKHR> &presentModes) {
    for (const auto &presentMode: presentModes) {
        // rendering mode
        if (presentMode == PresentModeKHR::eMailbox) {
            return presentMode;
        }
    }
    return PresentModeKHR::eFifo;
}

Extent2D
VulkanInit::chooseSwapchainExtent(uint32_t width, uint32_t height, const SurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        Extent2D extent = {width, height};

        extent.width = std::min(
                capabilities.maxImageExtent.width,
                std::max(capabilities.minImageExtent.width, extent.width)
        );

        extent.height = std::min(
                capabilities.maxImageExtent.height,
                std::max(capabilities.minImageExtent.height, extent.height)
        );

        return extent;
    }
}

SwapChainBundle
VulkanInit::createSwapchain(Device logicalDevice, PhysicalDevice physicalDevice, SurfaceKHR surface, int width,
                            int height, bool debug) {
    SwapChainSupportDetails support = querySwapchainSupport(physicalDevice, surface, debug);

    SurfaceFormatKHR format = chooseSwapchainSurfaceFormat(support.formats);

    PresentModeKHR presentMode = chooseSwapchainPresentMode(support.presentModes);

    Extent2D extent = chooseSwapchainExtent(width, height, support.capabilities);

    uint32_t imageCount;
    if (support.capabilities.maxImageCount == 0) {
        imageCount = std::min(int(support.capabilities.minImageCount + 1), 3);
    } else {
        imageCount = std::min(std::min((int)support.capabilities.minImageCount + 1, 3), (int)support.capabilities.maxImageCount);
    }
    
    SwapchainCreateInfoKHR createInfo;
    createInfo.flags = SwapchainCreateFlagsKHR();
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = ImageUsageFlagBits::eColorAttachment;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface, debug);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = SharingMode::eExclusive;
    }

    createInfo.preTransform = support.capabilities.currentTransform;
    createInfo.compositeAlpha = CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = SwapchainKHR(nullptr);

    SwapChainBundle bundle{};
    try {
        bundle.swapchain = logicalDevice.createSwapchainKHR(createInfo);
    } catch (const SystemError &err) {
        LOG_CRITICAL("Failed to create Swapchain!");
    }

    bundle.images = logicalDevice.getSwapchainImagesKHR(bundle.swapchain);
    bundle.format = format.format;
    bundle.extent = extent;

    return bundle;
}
