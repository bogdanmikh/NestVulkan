#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>
#include <sstream>
#include <filesystem>
#include <set>

#include "Nest/Application/Application.hpp"
#include "Nest/Logger/Logger.hpp"
#include "Nest/Renderer/Vulkan/Vulkan.hpp"
#include "Nest/Renderer/Vulkan/Device.hpp"
#include "Nest/Renderer/Vulkan/Instance.hpp"
#include "Nest/Renderer/Vulkan/Swapchain.hpp"
#include "Nest/Renderer/Vulkan/Logging.hpp"
#include "Nest/Renderer/Vulkan/Pipeline.hpp"

using namespace vk;

Vulkan::Vulkan()
        : instance(nullptr), debugMessenger(nullptr), logicalDevice(nullptr), physicalDevice(nullptr),
          graphicsQueue(nullptr), presentQueue(nullptr), swapchain(nullptr) {}

Vulkan::~Vulkan() {
    for (const auto &frame: swapchainFrames) {
        logicalDevice.destroyImageView(frame.imageView);
    }

    logicalDevice.destroyPipeline(pipeline);
    logicalDevice.destroyPipelineLayout(pipelineLayout);
    logicalDevice.destroyRenderPass(renderPass);

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
    makePipeline();
}

void Vulkan::makeInstance() {
    instance = InstanceInit::makeInstance(m_globalSettings.appName.c_str(), m_globalSettings.debugMode);
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

void Vulkan::makeDevice() {
    if (m_globalSettings.debugMode) {
        LOG_INFO("Choosing physical device...");
    }
    std::vector<PhysicalDevice> availableDevices = instance.enumeratePhysicalDevices();

    if (m_globalSettings.debugMode) {
        std::ostringstream message;
        message << "There are: " << availableDevices.size() << " physical devices available on the system";
        LOG_INFO("{}", message.str());
    }

    std::vector<std::array<bool, 5>> devicesTypes;
    for (const auto &device: availableDevices) {
        devicesTypes.emplace_back(DeviceInit::getDeviceProperties(device));
    }

    for (int numType = 0; numType < 5; ++numType) {
        for (int numDevice = 0; numDevice < availableDevices.size(); ++numDevice) {
            if (devicesTypes[numDevice][numType] &&
                DeviceInit::isSuitable(availableDevices[numDevice], m_globalSettings.debugMode)) {
                if (m_globalSettings.debugMode) {
                    VulkanLogging::logDeviceProperties(availableDevices[numDevice], devicesTypes[numDevice]);
                }
                physicalDevice = availableDevices[numDevice];
                logicalDevice = DeviceInit::createLogicalDevice(physicalDevice, surface, m_globalSettings.debugMode);
                auto queue = DeviceInit::getQueues(physicalDevice, logicalDevice, surface, m_globalSettings.debugMode);
                graphicsQueue = queue[0];
                presentQueue = queue[1];
                Swapchain::SwapChainBundle bundle = Swapchain::createSwapchain(logicalDevice, physicalDevice, surface,
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

void Vulkan::makePipeline() {
    PipelineInit::GraphicsPipelineInBundle specification;
    specification.device = logicalDevice;
    specification.swapchainExtent = swapchainExtent;
    specification.swapchainImageFormat = swapchainFormat;

    std::string localPath = std::filesystem::current_path().parent_path().parent_path().parent_path().string() + "/";
    specification.vertexFilepath = localPath + "Nest/res/Shaders/CompileShaders/vst.spv";
    specification.fragmentFilepath = localPath + "Nest/res/Shaders/CompileShaders/fst.spv";

    PipelineInit::GraphicsPipelineOutBundle output = PipelineInit::makeGraphicsPipeline(specification,
                                                                                        m_globalSettings.debugMode);
    pipeline = output.pipeline;
    pipelineLayout = output.layout;
    renderPass = output.renderPass;
}