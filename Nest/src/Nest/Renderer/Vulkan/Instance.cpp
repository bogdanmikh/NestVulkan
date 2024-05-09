#include <GLFW/glfw3.h>

#include "Nest/Renderer/Vulkan/Instance.hpp"
#include "Nest/Logger/Logger.hpp"

bool VulkanInit::supported(std::vector<const char*> &needExtensions, std::vector<const char*> &layers, bool debug) {
    std::vector<ExtensionProperties> supportedExtensions = enumerateInstanceExtensionProperties();

    if (debug) {
        // functions that Vulkan supports
        std::string message = "Device can support the following extensions:\n";
        for (auto &supportedExtension: supportedExtensions) {
            message += "\t";
            message += static_cast<std::string>(supportedExtension.extensionName);
            if (supportedExtension != supportedExtensions.back()) {
                message += "\n";
            }
        }
        LOG_INFO("{}", message);
    }

    std::string message;
    for (const auto &extension : needExtensions) {
        bool canSupport = false;
        for (const auto &supportedExtension: supportedExtensions) {
            if (strcmp(extension, supportedExtension.extensionName) == 0) {
                canSupport = true;
                if (debug) {
                    message += "\n\tExtension \"";
                    message += static_cast<std::string>(supportedExtension.extensionName);
                    message += "\" is supported";
                }
                break;
            }
        }
        if (!canSupport) {
            if (debug) {
                message += "\n\tExtension \"";
                message += static_cast<std::string>(extension);
                message += "\" is not supported";
            }
            LOG_INFO("{}", message);
            return false;
        }
    }
    if (debug) {
        LOG_INFO("{}", message);
    }
    message.clear();

    // check device can support layers
    std::vector<LayerProperties> supportedLayers = enumerateInstanceLayerProperties();
    if (debug) {
        message += "Device can support the following layers";
        for (const auto &supportedLayer: supportedLayers) {
            message += "\n\t";
            message += static_cast<std::string>(supportedLayer.layerName);
        }
        LOG_INFO("{}", message);
    }
    message.clear();
    for (const auto &layer: layers) {
        bool canSupport = false;
        for (const auto &supportedLayer: supportedLayers) {
            if (strcmp(layer, supportedLayer.layerName) == 0) {
                canSupport = true;
                if (debug) {
                    message += "\n\tExtension \"";
                    message += static_cast<std::string>(supportedLayer.layerName);
                    message += "\" is supported";
                }
                break;
            }
        }
        if (!canSupport) {
            if (debug) {
                message += "\n\tExtension \"";
                message += static_cast<std::string>(layer);
                message += "\" is not supported";
                LOG_INFO("{}", message);
            }
            return false;
        }
    }
    if (debug) {
        LOG_INFO("{}", message);
    }

    return true;
}

Instance VulkanInit::makeInstance(const char *appName, bool debugMode) {
    if (debugMode) {
        LOG_INFO("Make instance...");
    }
    uint32_t version;
    vkEnumerateInstanceVersion(&version);
    if (debugMode) {
        LOG_INFO("System can support vulkan version: {}. {}.{}.{}",
                 VK_API_VERSION_VARIANT(version), VK_API_VERSION_MAJOR(version), VK_API_VERSION_MINOR(version),
                 VK_API_VERSION_PATCH(version));
    }
    // patch = 0
    version &= ~(0xFFFU);

    ApplicationInfo appInfo;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = version;
    appInfo.pEngineName = "Nest";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 1, 0);
    appInfo.apiVersion = version;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtension;
    // the necessary extensions that GLFW needs to work with Vulkan
    glfwExtension = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtension, glfwExtension + glfwExtensionCount);
    if (debugMode) {
        // add extension utils for debug
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    if (debugMode) {
        std::string message = "Extension to be requested\n";

        for (int i = 0; i < extensions.size() - 1; ++i) {
            message += "\t";
            message += extensions[i];
            message += "\n";
        }
        message += "\t";
        message += extensions.back();

        LOG_INFO("{}", message);
    }

    std::vector<const char*> layers;
    if (debugMode) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    // check can support GLFW this device for Vulkan
    if (!VulkanInit::supported(extensions, layers, debugMode)) {
        if (debugMode) {
            LOG_ERROR("Device not supported need extensions");
        }
        return nullptr;
    }

    InstanceCreateInfo createInfo;
    createInfo.flags = InstanceCreateFlags() | InstanceCreateFlagBits::eEnumeratePortabilityKHR;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    try {
        return createInstance(createInfo);
    } catch (const SystemError &err) {
        if (debugMode) {
            LOG_ERROR("Failed to create vulkan Instance! {}", err.what());
        }
        return nullptr;
    }
}

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

DebugUtilsMessengerEXT VulkanInit::makeDebugMessenger(const Instance& instance, const DispatchLoaderDynamic& dld) {
    DebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo.flags = DebugUtilsMessengerCreateFlagsEXT();
    createInfo.messageSeverity = DebugUtilsMessageSeverityFlagBitsEXT::eWarning | DebugUtilsMessageSeverityFlagBitsEXT::eError;
    createInfo.messageType = DebugUtilsMessageTypeFlagBitsEXT::eGeneral | DebugUtilsMessageTypeFlagBitsEXT::eValidation | DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    createInfo.pfnUserCallback = &debugCallback;
    createInfo.pUserData = nullptr;
    return instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dld);
}
