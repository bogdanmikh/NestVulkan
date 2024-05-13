#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>
#include <set>

#include "Nest/Application/Application.hpp"
#include "Nest/Logger/Logger.hpp"
#include "Nest/Renderer/Vulkan/Vulkan.hpp"
#include "Nest/Renderer/Vulkan/Device.hpp"
#include "Nest/Renderer/Vulkan/Instance.hpp"
#include "Nest/Renderer/Vulkan/Swapchain.hpp"
#include "Nest/Renderer/Vulkan/Logging.hpp"

using namespace vk;

Vulkan::Vulkan()
        : instance(nullptr)
        , debugMessenger(nullptr)
        , logicalDevice(nullptr)
        , physicalDevice(nullptr)
        , graphicsQueue(nullptr)
        , presentQueue(nullptr)
        , swapchain(nullptr) {}

Vulkan::~Vulkan() {
    for (const auto &frame: swapchainFrames) {
        logicalDevice.destroyImageView(frame.imageView);
    }

    logicalDevice.destroySwapchainKHR(swapchain);
    logicalDevice.destroy();
    instance.destroySurfaceKHR(surface);
    if (m_globalSettings.debugMode) {
        instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dld);
    }
    instance.destroy();
}

void Vulkan::init(const GlobalSettings &globalSettings) {
    m_globalSettings = globalSettings;
    makeInstance();
    makeDevice();
}

void Vulkan::makeInstance() {
    instance = VulkanInit::makeInstance(m_globalSettings.appName.c_str(), m_globalSettings.debugMode);
    dld = DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
    if (m_globalSettings.debugMode) {
        debugMessenger = VulkanLogging::makeDebugMessenger(instance, dld);
    }
    VkSurfaceKHR cStyleSurface;
    auto *window = static_cast<GLFWwindow *>(Application::getInstance()->getWindow()->getNativeHandle());
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &cStyleSurface);
    if (result != VK_SUCCESS) {
        if (m_globalSettings.debugMode) {
            LOG_CRITICAL("Failed to abstract the glfw surface for Vulkan");
        }
    } else if (m_globalSettings.debugMode) {
        LOG_INFO("Successfully abstracted glfw surface for Vulkan");
    }
    surface = cStyleSurface;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData) {
    std::ostringstream stringStream;
    stringStream << "Validation layer: ";
    stringStream << pCallbackData->pMessage;
    LOG_ERROR("{}", stringStream.str());

    return VK_FALSE;
}

void Vulkan::makeDevice() {
    if (m_globalSettings.debugMode) {
        LOG_INFO("Choosing physical device...");
    }
    std::vector<PhysicalDevice> availableDevices = instance.enumeratePhysicalDevices();

    if (m_globalSettings.debugMode) {
        std::ostringstream stringStream;
        stringStream << "There are: " << availableDevices.size() << " physical devices available on the system";
        LOG_INFO("{}", stringStream.str());
    }

    std::vector<std::array<bool, 5>> devicesTypes;
    for (const auto &device: availableDevices) {
        devicesTypes.emplace_back(VulkanInit::getDeviceProperties(device));
    }

    for (int numType = 0; numType < 5; ++numType) {
        for (int numDevice = 0; numDevice < availableDevices.size(); ++numDevice) {
            if (devicesTypes[numDevice][numType] &&
                VulkanInit::isSuitable(availableDevices[numDevice], m_globalSettings.debugMode)) {
                if (m_globalSettings.debugMode) {
                    VulkanLogging::logDeviceProperties(availableDevices[numDevice], devicesTypes[numDevice]);
                }
                physicalDevice = availableDevices[numDevice];
                logicalDevice = VulkanInit::createLogicalDevice(physicalDevice, surface, m_globalSettings.debugMode);
                auto queue = VulkanInit::getQueues(physicalDevice, logicalDevice, surface, m_globalSettings.debugMode);
                graphicsQueue = queue[0];
                presentQueue = queue[1];
                VulkanInit::SwapChainBundle bundle = VulkanInit::createSwapchain(logicalDevice, physicalDevice, surface,
                                                                     m_globalSettings.resolutionX,
                                                                     m_globalSettings.resolutionY,
                                                                     m_globalSettings.debugMode);
                swapchain = bundle.swapchain;
                swapchainFrames = bundle.frames;
                swapchainFormat = bundle.format;
                swapchainExtent = bundle.extent;
                return;
            }
        }
    }
}