#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <vector>

#include "Nest/Renderer/Vulkan/Vulkan.hpp"
#include "Nest/Logger/Logger.hpp"
#include "Nest/Renderer/Vulkan/Device.hpp"

using namespace vk;

Vulkan::Vulkan() : debugMessenger(nullptr) {}

Vulkan::~Vulkan() {
    vulkanInstance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dld);
    vulkanInstance.destroy();
}

void Vulkan::init(bool debug, const char* appName) {
    makeInstance(debug, appName);
    glfwInitVulkanLoader(vkGetInstanceProcAddr);
    dld = DispatchLoaderDynamic(vulkanInstance, vkGetInstanceProcAddr);
    if (debug) {
        makeDebugMessenger();
    }
    makeDevice(debug);
}

bool Vulkan::supported(std::vector<const char*> &needExtensions, std::vector<const char*> &layers, bool debug) {
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

void Vulkan::makeInstance(bool debug, const char *appName) {
    if (debug) {
        LOG_INFO("Make instance...");
    }
    uint32_t version;
    vkEnumerateInstanceVersion(&version);
    if (debug) {
        LOG_INFO("System can support vulkan version: {}. {}.{}.{}",
                 VK_API_VERSION_VARIANT(version), VK_API_VERSION_MAJOR(version), VK_API_VERSION_MINOR(version),
                 VK_API_VERSION_PATCH(version));
    }
    // patch = 0
//    version &= ~(0xFFFU);
    version = VK_API_VERSION_1_1;

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
    if (debug) {
        // add extension utils for debug
        extensions.push_back("VK_EXT_debug_utils");
    }
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    if (debug) {
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
    if (debug) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    // check can support GLFW this device for Vulkan
    if (!supported(extensions, layers, debug)) {
        if (debug) {
            LOG_ERROR("Device not supported need extensions");
        }
        vulkanInstance = nullptr;
        return;
    }

    InstanceCreateInfo createInfo;
    createInfo.flags = InstanceCreateFlags() | InstanceCreateFlagBits::eEnumeratePortabilityKHR;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    try {
        vulkanInstance = createInstance(createInfo);
    } catch (const SystemError &err) {
        if (debug) {
            LOG_ERROR("Failed to create vulkan Instance! {}", err.what());
        }
        vulkanInstance = nullptr;
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
) {
    std::string message = "Validation layer: ";
    message += pCallbackData->pMessage;
    if (messageType == VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        LOG_WARN("{}", message);
    } else {
        LOG_ERROR("{}", message);
    }

    return VK_FALSE;
}

void Vulkan::makeDebugMessenger() {
    DebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo.flags = DebugUtilsMessengerCreateFlagsEXT();
    createInfo.messageSeverity = DebugUtilsMessageSeverityFlagBitsEXT::eWarning | DebugUtilsMessageSeverityFlagBitsEXT::eError;
    createInfo.messageType = DebugUtilsMessageTypeFlagBitsEXT::eGeneral | DebugUtilsMessageTypeFlagBitsEXT::eValidation | DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    createInfo.pfnUserCallback = &debugCallback;
    createInfo.pUserData = nullptr;
    debugMessenger = vulkanInstance.createDebugUtilsMessengerEXT(createInfo, nullptr, dld);
}

void Vulkan::makeDevice(bool debug) {
    if (debug) {
        LOG_INFO("Choosing physical device...");
    }
    std::vector<PhysicalDevice> availableDevices = vulkanInstance.enumeratePhysicalDevices();

    if (debug) {
        std::string message = "There are: " + std::to_string(availableDevices.size()) + " physical devices available on the system";
        LOG_INFO("{}", message);
    }
    for (const auto &device: availableDevices) {
        if (debug) {
            logDeviceProperties(device);
        }
        if (isSuitable(device, debug)) {
            physicalDevice = device;
            return;
        }
    }
}
