#include "Nest/Logger/Logger.hpp"
#include "Nest/Renderer/Vulkan/Swapchain.hpp"
#include "Nest/Renderer/Vulkan/Logging.hpp"
#include "Nest/Renderer/Vulkan/QueueFamilies.hpp"

Swapchain::SwapChainSupportDetails
Swapchain::querySwapchainSupport(const PhysicalDevice &device, const SurfaceKHR &surface, bool debug) {
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

SurfaceFormatKHR Swapchain::chooseSwapchainSurfaceFormat(const std::vector<SurfaceFormatKHR> &formats) {
    for (const auto &format: formats) {
        // color format an 0-255, non linear
        if (format.format == Format::eB8G8R8A8Unorm
            && format.colorSpace == ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }
    return formats[0];
}

PresentModeKHR Swapchain::chooseSwapchainPresentMode(const std::vector<PresentModeKHR> &presentModes) {
    for (const auto &presentMode: presentModes) {
        // rendering mode
        if (presentMode == PresentModeKHR::eMailbox) {
            return presentMode;
        }
    }
    return PresentModeKHR::eFifo;
}

Extent2D
Swapchain::chooseSwapchainExtent(uint32_t width, uint32_t height, const SurfaceCapabilitiesKHR &capabilities) {
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

Swapchain::SwapChainBundle
Swapchain::createSwapchain(Device logicalDevice, PhysicalDevice physicalDevice, SurfaceKHR surface, int width,
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

    QueueFamilies::QueueFamilyIndices indices = QueueFamilies::findQueueFamilies(physicalDevice, surface, debug);
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

    std::vector<Image> images = logicalDevice.getSwapchainImagesKHR(bundle.swapchain);
    bundle.frames.resize(images.size());
    for (int i = 0; i < images.size(); ++i) {
        ImageViewCreateInfo imageViewInfo;
        imageViewInfo.image = images[i];
        imageViewInfo.viewType = ImageViewType::e2D;
        // We can change the order of the colors
        imageViewInfo.components.r = ComponentSwizzle::eIdentity;
        imageViewInfo.components.g = ComponentSwizzle::eIdentity;
        imageViewInfo.components.b = ComponentSwizzle::eIdentity;
        imageViewInfo.components.a = ComponentSwizzle::eIdentity;

        imageViewInfo.subresourceRange.aspectMask = ImageAspectFlagBits::eColor;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;
        imageViewInfo.format = format.format;

        bundle.frames[i].image = images[i];
        bundle.frames[i].imageView = logicalDevice.createImageView(imageViewInfo);
    }
    bundle.format = format.format;
    bundle.extent = extent;

    return bundle;
}
