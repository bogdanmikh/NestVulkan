#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

#include "Nest/Application/Application.hpp"
#include "Nest/Logger/Logger.hpp"
#include "Nest/Renderer/Vulkan/Vulkan.hpp"
#include "Nest/Renderer/Vulkan/Device.hpp"
#include "Nest/Renderer/Vulkan/Instance.hpp"

using namespace vk;

Vulkan::Vulkan()
    : instance(nullptr)
    , debugMessenger(nullptr)
    , logicalDevice(nullptr)
    , physicalDevice(nullptr)
    , graphicsQueue(nullptr)
    , presentQueue(nullptr)
    , debugMode(true) {}

Vulkan::~Vulkan() {
    logicalDevice.destroy();
    instance.destroySurfaceKHR(surface);
    if (debugMode) {
        instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dld);
    }
    instance.destroy();
}

void Vulkan::init(bool debug, const char* appName) {
    debugMode = debug;
    makeInstance(appName);
    makeDevice();
}

void Vulkan::makeInstance(const char *appName) {
    instance = VulkanInit::makeInstance(appName, debugMode);
    dld = DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
    if (debugMode) {
        debugMessenger = VulkanInit::makeDebugMessenger(instance, dld);
    }
    VkSurfaceKHR cStyleSurface;
    auto* window = static_cast<GLFWwindow*>(Application::getInstance()->getWindow()->getNativeHandle());
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &cStyleSurface);
    if (result != VK_SUCCESS) {
        if (debugMode) {
            LOG_CRITICAL("Failed to abstract the glfw surface for Vulkan");
        }
    } else if (debugMode) {
        LOG_INFO("Successfully abstracted glfw surface for Vulkan");
    }
    surface = cStyleSurface;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
    std::string message = "Validation layer: ";
    message += pCallbackData->pMessage;
    if (messageType == VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        LOG_WARN("{}", message);
    } else {
        LOG_ERROR("{}", message);
    }

    return VK_FALSE;
}
void Vulkan::makeDevice() {
    if (debugMode) {
        LOG_INFO("Choosing physical device...");
    }
    std::vector<PhysicalDevice> availableDevices = instance.enumeratePhysicalDevices();

    if (debugMode) {
        std::string message = "There are: " + std::to_string(availableDevices.size()) + " physical devices available on the system";
        LOG_INFO("{}", message);
    }
    for (const auto &device: availableDevices) {
        if (debugMode) {
            DeviceInit::logDeviceProperties(device);
        }
        if (DeviceInit::isSuitable(device, debugMode)) {
            physicalDevice = device;
            logicalDevice = DeviceInit::createLogicalDevice(physicalDevice, surface, debugMode);
            auto queue = DeviceInit::getQueues(physicalDevice, logicalDevice, surface, debugMode);
            graphicsQueue = queue[0];
            presentQueue = queue[1];
            return;
        }
    }
}