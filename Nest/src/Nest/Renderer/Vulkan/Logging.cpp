#include <vector>

#include "Nest/Logger/Logger.hpp"
#include "Nest/Renderer/Vulkan/Logging.hpp"

using namespace vk;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
    std::string message = "Validation layer: ";
    message += pCallbackData->pMessage;
    LOG_ERROR("{}", message);
    return VK_FALSE;
}

DebugUtilsMessengerEXT VulkanLogging::makeDebugMessenger(const Instance& instance, const DispatchLoaderDynamic& dld) {
    DebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo.flags = DebugUtilsMessengerCreateFlagsEXT();
    createInfo.messageSeverity = DebugUtilsMessageSeverityFlagBitsEXT::eWarning | DebugUtilsMessageSeverityFlagBitsEXT::eError;
    createInfo.messageType = DebugUtilsMessageTypeFlagBitsEXT::eGeneral | DebugUtilsMessageTypeFlagBitsEXT::eValidation | DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    createInfo.pfnUserCallback = &debugCallback;
    createInfo.pUserData = nullptr;
    return instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dld);
}


void VulkanLogging::logDeviceProperties(const PhysicalDevice &device, const std::array<bool, 5> &types) {
    std::ostringstream stringStream;
    stringStream << "\n\tDevice name: " << static_cast<const char*>(device.getProperties().deviceName) << "\n\tDevice type:";
    std::array<const char*, 5> allTypes{
            "Discrete GPU", "Integrated GPU", "Virtual GPU", "CPU", "Other"
    };
    for (int i = 0; i < 5; ++i) {
        if (types[i]) {
            stringStream << "\n\t" << allTypes[i];
        }
    }
    LOG_INFO("{}", stringStream.str());
}

std::vector<const char *> VulkanLogging::logTransformBits(const SurfaceTransformFlagsKHR &bits) {
    std::vector<const char*> result;
    if (bits & SurfaceTransformFlagBitsKHR::eIdentity) {
        result.emplace_back("identity");
    }
    if (bits & SurfaceTransformFlagBitsKHR::eRotate90) {
        result.emplace_back("90 degree rotation");
    }
    if (bits & SurfaceTransformFlagBitsKHR::eRotate180) {
        result.emplace_back("180 degree rotation");
    }
    if (bits & SurfaceTransformFlagBitsKHR::eRotate270) {
        result.emplace_back("270 degree rotation");
    }
    if (bits & SurfaceTransformFlagBitsKHR::eHorizontalMirror) {
        result.emplace_back("horizontal mirror");
    }
    if (bits & SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90) {
        result.emplace_back("horizontal mirror, then 90 degree rotation");
    }
    if (bits & SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate180) {
        result.emplace_back("horizontal mirror, then 180 degree rotation");
    }
    if (bits & SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270) {
        result.emplace_back("horizontal mirror, then 270 degree rotation");
    }
    if (bits & SurfaceTransformFlagBitsKHR::eInherit) {
        result.emplace_back("inherited");
    }

    return result;
}


std::vector<const char*> VulkanLogging::logAlphaCompositeBits(const CompositeAlphaFlagsKHR& bits) {
    std::vector<const char*> result;
    if (bits & CompositeAlphaFlagBitsKHR::eOpaque) {
        result.emplace_back("opaque (alpha ignored)");
    }
    if (bits & CompositeAlphaFlagBitsKHR::ePreMultiplied) {
        result.emplace_back("pre multiplied (alpha expected to already be multiplied in image)");
    }
    if (bits & CompositeAlphaFlagBitsKHR::ePostMultiplied) {
        result.emplace_back("post multiplied (alpha will be applied during composition)");
    }
    if (bits & CompositeAlphaFlagBitsKHR::eInherit) {
        result.emplace_back("inherited");
    }

    return result;
}

std::vector<const char *> VulkanLogging::logImageUsageBits(const ImageUsageFlags& bits) {
    std::vector<const char *> result;
    if (bits & ImageUsageFlagBits::eTransferSrc) {
        result.push_back("transfer src: image can be used as the source of a transfer command.");
    }
    if (bits & ImageUsageFlagBits::eTransferDst) {
        result.push_back("transfer dst: image can be used as the destination of a transfer command.");
    }
    if (bits & ImageUsageFlagBits::eSampled) {
        result.push_back("sampled: image can be used to create a VkImageView suitable for occupying a \
VkDescriptorSet slot either of type VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE or \
VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, and be sampled by a shader.");
    }
    if (bits & ImageUsageFlagBits::eStorage) {
        result.push_back("storage: image can be used to create a VkImageView suitable for occupying a \
VkDescriptorSet slot of type VK_DESCRIPTOR_TYPE_STORAGE_IMAGE.");
    }
    if (bits & ImageUsageFlagBits::eColorAttachment) {
        result.push_back("color attachment: image can be used to create a VkImageView suitable for use as \
a color or resolve attachment in a VkFramebuffer.");
    }
    if (bits & ImageUsageFlagBits::eDepthStencilAttachment) {
        result.push_back("depth/stencil attachment: image can be used to create a VkImageView \
suitable for use as a depth/stencil or depth/stencil resolve attachment in a VkFramebuffer.");
    }
    if (bits & ImageUsageFlagBits::eTransientAttachment) {
        result.push_back("transient attachment: implementations may support using memory allocations \
with the VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT to back an image with this usage. This \
bit can be set for any image that can be used to create a VkImageView suitable for use as \
a color, resolve, depth/stencil, or input attachment.");
    }
    if (bits & ImageUsageFlagBits::eInputAttachment) {
        result.push_back("input attachment: image can be used to create a VkImageView suitable for \
occupying VkDescriptorSet slot of type VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; be read from \
a shader as an input attachment; and be used as an input attachment in a framebuffer.");
    }
    if (bits & ImageUsageFlagBits::eFragmentDensityMapEXT) {
        result.push_back("fragment density map: image can be used to create a VkImageView suitable \
for use as a fragment density map image.");
    }
    if (bits & ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR) {
        result.push_back("fragment shading rate attachment: image can be used to create a VkImageView \
suitable for use as a fragment shading rate attachment or shading rate image");
    }
    return result;
}

const char *VulkanLogging::logPresentMode(const PresentModeKHR &presentMode) {
    if (presentMode == PresentModeKHR::eImmediate) {
        return "immediate: the presentation engine does not wait for a vertical blanking period \
to update the current image, meaning this mode may result in visible tearing. No internal \
queuing of presentation requests is needed, as the requests are applied immediately.";
    }
    if (presentMode == PresentModeKHR::eMailbox) {
        return "mailbox: the presentation engine waits for the next vertical blanking period \
to update the current image. Tearing cannot be observed. An internal single-entry queue is \
used to hold pending presentation requests. If the queue is full when a new presentation \
request is received, the new request replaces the existing entry, and any images associated \
with the prior entry become available for re-use by the application. One request is removed \
from the queue and processed during each vertical blanking period in which the queue is non-empty.";
    }
    if (presentMode == PresentModeKHR::eFifo) {
        return "fifo: the presentation engine waits for the next vertical blanking \
period to update the current image. Tearing cannot be observed. An internal queue is used to \
hold pending presentation requests. New requests are appended to the end of the queue, and one \
request is removed from the beginning of the queue and processed during each vertical blanking \
period in which the queue is non-empty. This is the only value of presentMode that is required \
to be supported.";
    }
    if (presentMode == PresentModeKHR::eFifoRelaxed) {
        return "relaxed fifo: the presentation engine generally waits for the next vertical \
blanking period to update the current image. If a vertical blanking period has already passed \
since the last update of the current image then the presentation engine does not wait for \
another vertical blanking period for the update, meaning this mode may result in visible tearing \
in this case. This mode is useful for reducing visual stutter with an application that will \
mostly present a new image before the next vertical blanking period, but may occasionally be \
late, and present a new image just after the next vertical blanking period. An internal queue \
is used to hold pending presentation requests. New requests are appended to the end of the queue, \
and one request is removed from the beginning of the queue and processed during or after each \
vertical blanking period in which the queue is non-empty.";
    }
    if (presentMode == PresentModeKHR::eSharedDemandRefresh) {
        return "shared demand refresh: the presentation engine and application have \
concurrent access to a single image, which is referred to as a shared presentable image. \
The presentation engine is only required to update the current image after a new presentation \
request is received. Therefore the application must make a presentation request whenever an \
update is required. However, the presentation engine may update the current image at any point, \
meaning this mode may result in visible tearing.";
    }
    if (presentMode == PresentModeKHR::eSharedContinuousRefresh) {
        return "shared continuous refresh: the presentation engine and application have \
concurrent access to a single image, which is referred to as a shared presentable image. The \
presentation engine periodically updates the current image on its regular refresh cycle. The \
application is only required to make one initial presentation request, after which the \
presentation engine must update the current image without any need for further presentation \
requests. The application can indicate the image contents have been updated by making a \
presentation request, but this does not guarantee the timing of when it will be updated. \
This mode may result in visible tearing if rendering to the image is not timed correctly.";
    }
    return "none/undefined";
}