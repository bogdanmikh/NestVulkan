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
#include "Nest/Renderer/Vulkan/Sync.hpp"
#include "Nest/Renderer/Vulkan/Commands.hpp"
#include "Nest/Renderer/Vulkan/Framebuffer.hpp"

using namespace vk;

static std::string localPath = std::filesystem::current_path().parent_path().parent_path().parent_path().string() + "/";

Vulkan::Vulkan()
        : instance(nullptr), debugMessenger(nullptr), logicalDevice(nullptr), physicalDevice(nullptr),
          graphicsQueue(nullptr), presentQueue(nullptr), swapchain(nullptr) {}

Vulkan::~Vulkan() {
    logicalDevice.waitIdle();
    logicalDevice.destroyCommandPool(commandPool);

    logicalDevice.destroyPipeline(pipeline);
    logicalDevice.destroyPipelineLayout(pipelineLayout);
    logicalDevice.destroyRenderPass(renderPass);

    for (const auto &frame: swapchainFrames) {
        logicalDevice.destroyImageView(frame.imageView);
        logicalDevice.destroyFramebuffer(frame.framebuffer);
        logicalDevice.destroyFence(frame.inFlight);
        logicalDevice.destroySemaphore(frame.imageAvailable);
        logicalDevice.destroySemaphore(frame.renderFinished);
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
    makePipeline();
    finalizeSetup();
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
                maxFramesInFlight = static_cast<int>(swapchainFrames.size());
                frameNumber = 0;
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
    specification.vertexFilepath = localPath + "Nest/res/Shaders/CompileShaders/vst.spv";
    specification.fragmentFilepath = localPath + "Nest/res/Shaders/CompileShaders/fst.spv";

    PipelineInit::GraphicsPipelineOutBundle output = PipelineInit::makeGraphicsPipeline(specification,
                                                                                        m_globalSettings.debugMode);
    pipeline = output.pipeline;
    pipelineLayout = output.layout;
    renderPass = output.renderPass;
}

void Vulkan::finalizeSetup() {
    FrameBufferInit::FramebufferInput framebufferInput;
    framebufferInput.device = logicalDevice;
    framebufferInput.renderPass = renderPass;
    framebufferInput.swapchainExtent = swapchainExtent;
    FrameBufferInit::makeFrameBuffers(framebufferInput, swapchainFrames, m_globalSettings.debugMode);

    commandPool = Commands::makeCommandPool(logicalDevice, physicalDevice, surface, m_globalSettings.debugMode);

    Commands::CommandBufferInputChunk commandBufferInput = {logicalDevice, commandPool, swapchainFrames};

    mainCommandBuffer = Commands::makeCommandBuffers(commandBufferInput, m_globalSettings.debugMode);

    for (auto &frame: swapchainFrames) {
        frame.inFlight = Sync::makeFence(logicalDevice, m_globalSettings.debugMode);
        frame.imageAvailable = Sync::makeSemaphore(logicalDevice, m_globalSettings.debugMode);
        frame.renderFinished = Sync::makeSemaphore(logicalDevice, m_globalSettings.debugMode);
    }
}

void Vulkan::recordDrawCommands(const CommandBuffer &commandBuffer, uint32_t imageIndex) {
    CommandBufferBeginInfo beginInfo;
    try {
        commandBuffer.begin(beginInfo);
    } catch (const SystemError &err) {
        if (m_globalSettings.debugMode) {
            LOG_ERROR("Failed to begin recording command buffer!\n{}", err.what());
        }
    }

    RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapchainFrames[imageIndex].framebuffer;
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = swapchainExtent;

    std::array<float, 4> clearColorVal = {0.8, 1., 0., 1.};
    ClearValue clearColor(clearColorVal);
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    commandBuffer.beginRenderPass(&renderPassInfo, SubpassContents::eInline);
    commandBuffer.bindPipeline(PipelineBindPoint::eGraphics, pipeline);

    commandBuffer.draw(6, 1, 0, 0);

    commandBuffer.endRenderPass();

    try {
        commandBuffer.end();
    } catch (const SystemError &err) {
        if (m_globalSettings.debugMode) {
            LOG_ERROR("Failed to record command buffer!\n{}", err.what());
        }
    }
}

void Vulkan::render() {
    auto currentFrame = swapchainFrames[frameNumber];
    logicalDevice.waitForFences(1, &currentFrame.inFlight, VK_TRUE, UINT64_MAX);
    logicalDevice.resetFences(1, &currentFrame.inFlight);

//     acquireNextImageKHR(SwapChainKHR, timeout, semaphore_to_signal, fence)
    uint32_t imageIndex = logicalDevice.acquireNextImageKHR(swapchain, UINT64_MAX, currentFrame.imageAvailable, nullptr).value;

    CommandBuffer commandBuffer = currentFrame.commandBuffer;

    commandBuffer.reset();

    recordDrawCommands(commandBuffer, imageIndex);

    SubmitInfo submitInfo;

    Semaphore waitSemaphores[] = { currentFrame.imageAvailable };
    PipelineStageFlags waitStages[] = { PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    Semaphore signalSemaphores[] = { currentFrame.renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    try {
        graphicsQueue.submit(submitInfo, currentFrame.inFlight);
    } catch (const SystemError &err) {
        if (m_globalSettings.debugMode) {
            LOG_ERROR("Failed to submit draw command buffer!\n{}", err.what());
        }
    }

    PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    SwapchainKHR swapChains[] = { swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    presentQueue.presentKHR(presentInfo);

    frameNumber = (frameNumber + 1) % maxFramesInFlight;
}