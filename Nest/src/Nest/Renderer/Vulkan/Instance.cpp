#include <GLFW/glfw3.h>

#include "Nest/Renderer/Vulkan/Instance.hpp"
#include "Nest/Logger/Logger.hpp"

bool InstanceInit::supported(std::vector<const char*> &needExtensions, std::vector<const char*> &layers, bool debug) {
    std::vector<ExtensionProperties> supportedExtensions = enumerateInstanceExtensionProperties();

    if (debug) {
        // functions that Vulkan supports
        std::ostringstream message;
        message << "Device can support the following extensions:";
        for (auto &supportedExtension: supportedExtensions) {
            message << "\n\t" << static_cast<std::string>(supportedExtension.extensionName);
        }
        LOG_INFO("{}", message.str());
    }

    std::ostringstream message;
    for (const auto &extension : needExtensions) {
        bool canSupport = false;
        for (const auto &supportedExtension: supportedExtensions) {
            if (strcmp(extension, supportedExtension.extensionName) == 0) {
                canSupport = true;
                if (debug) {
                    message << "\n\tExtension \"" << static_cast<std::string>(supportedExtension.extensionName) << "\" is supported";
                }
                break;
            }
        }
        if (!canSupport) {
            if (debug) {
                message << "\n\tExtension \"" << static_cast<std::string>(extension) << "\" is not supported";
            }
            LOG_INFO("{}", message.str());
            return false;
        }
    }
    if (debug) {
        LOG_INFO("{}", message.str());
    }
    message.clear();

    // check device can support layers
    std::vector<LayerProperties> supportedLayers = enumerateInstanceLayerProperties();
    if (debug) {
        message << "\n\tDevice can support the following layers";
        for (const auto &supportedLayer: supportedLayers) {
            message << "\n\t" << static_cast<std::string>(supportedLayer.layerName);
        }
        LOG_INFO("{}", message.str());
    }
    message.str().clear();
    for (const auto &layer: layers) {
        bool canSupport = false;
        for (const auto &supportedLayer: supportedLayers) {
            if (strcmp(layer, supportedLayer.layerName) == 0) {
                canSupport = true;
                if (debug) {
                    message << "\n\tExtension \"" << static_cast<std::string>(supportedLayer.layerName) << "\" is supported";
                }
                break;
            }
        }
        if (!canSupport) {
            if (debug) {
                message << "\n\tExtension \"" << static_cast<std::string>(layer) << "\" is not supported";
                LOG_INFO("{}", message.str());
            }
            return false;
        }
    }
    if (debug) {
        LOG_INFO("{}", message.str());
    }
    return true;
}

Instance InstanceInit::makeInstance(const char *appName, bool debugMode) {
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
        std::ostringstream message;
        message << "Extension to be requested\n";

        for (int i = 0; i < extensions.size() - 1; ++i) {
            message << "\t" << extensions[i] << "\n";
        }
        message << "\t" << extensions.back();

        LOG_INFO("{}", message.str());
    }

    std::vector<const char*> layers;
    if (debugMode) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    // check can support GLFW this device for Vulkan
    if (!InstanceInit::supported(extensions, layers, debugMode)) {
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