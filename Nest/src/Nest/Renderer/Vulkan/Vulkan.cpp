#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>
#include <sstream>
#include <filesystem>
#include <set>

#include "Nest/Logger/Logger.hpp"
#include "Nest/Settings/SettingsLog.hpp"
#include "Nest/Application/Application.hpp"
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

    cleanupSwapchain();

    logicalDevice.destroy();
    instance.destroySurfaceKHR(surface);
    if (VK_PRINT_INSTANCE_INFO) {
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
    instance = makeInstanceVulkan(m_globalSettings.appName.c_str());
    dld = DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
    bool debug = VK_PRINT_INSTANCE_INFO;
    if (debug) {
        debugMessenger = makeDebugMessenger(instance, dld);
    }
    VkSurfaceKHR cStyleSurface;
    auto *window = static_cast<GLFWwindow *>(Application::getInstance()->getWindow()->getNativeHandle());
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &cStyleSurface);
    if (result != VK_SUCCESS) {
        if (debug) {
            LOG_CRITICAL("Failed to abstract the glfw surface for Vulkan");
        }
    } else if (debug) {
        LOG_INFO("Successfully abstracted GLFW surface for Vulkan");
    }
    surface = cStyleSurface;
}

void Vulkan::makeDevice() {
    bool debug = VK_PRINT_DEVICE_INFO;
    if (debug) {
        LOG_INFO("Choosing physical device...");
    }
    std::vector<PhysicalDevice> availableDevices = instance.enumeratePhysicalDevices();

    if (debug) {
        std::ostringstream message;
        message << "There are: " << availableDevices.size() << " physical devices available on the system";
        LOG_INFO("{}", message.str());
    }

    std::vector<std::array<bool, 5>> devicesTypes;
    for (const auto &device: availableDevices) {
        devicesTypes.emplace_back(getDeviceProperties(device));
    }

    for (int numType = 0; numType < 5; ++numType) {
        for (int numDevice = 0; numDevice < availableDevices.size(); ++numDevice) {
            if (devicesTypes[numDevice][numType] &&
                isSuitable(availableDevices[numDevice])) {
                if (VK_PRINT_SELECTED_DEVICE_INFO) {
                    logDeviceProperties(availableDevices[numDevice], devicesTypes[numDevice]);
                }
                physicalDevice = availableDevices[numDevice];
                logicalDevice = createLogicalDevice(physicalDevice, surface, m_globalSettings.debugMode);
                auto queue = getQueues(physicalDevice, logicalDevice, surface, m_globalSettings.debugMode);
                graphicsQueue = queue[0];
                presentQueue = queue[1];
                makeSwapchain();
                frameNumber = 0;
                return;
            }
        }
    }
}

void Vulkan::makeSwapchain() {
    SwapChainBundle bundle = createSwapchain(logicalDevice, physicalDevice, surface,
                                 m_globalSettings.resolutionX,
                                 m_globalSettings.resolutionY);
    swapchain = bundle.swapchain;
    swapchainFrames = bundle.frames;
    swapchainFormat = bundle.format;
    swapchainExtent = bundle.extent;
    maxFramesInFlight = static_cast<int>(swapchainFrames.size());
}

void Vulkan::recreateSwapchain() {
//    m_globalSettings.resolutionX = 0;
//    m_globalSettings.resolutionY = 0;
    while (m_globalSettings.resolutionX == 0 || m_globalSettings.resolutionY == 0) {
        glfwGetFramebufferSize((GLFWwindow *) Application::getInstance()->getWindow()->getNativeHandle(),
                               &m_globalSettings.resolutionX, &m_globalSettings.resolutionY);
        glfwWaitEvents();
    }
    logicalDevice.waitIdle();

    SwapChainBundle bundle = createSwapchain(logicalDevice, physicalDevice, surface,
                                 m_globalSettings.resolutionX,
                                 m_globalSettings.resolutionY, &swapchain);
    cleanupSwapchain();
    swapchain = bundle.swapchain;
    swapchainFrames = bundle.frames;
    swapchainFormat = bundle.format;
    swapchainExtent = bundle.extent;
    maxFramesInFlight = static_cast<int>(swapchainFrames.size());
    makeFramebuffer();
    makeFrameSync();
    commandPool = makeCommandPool(logicalDevice, physicalDevice, surface, m_globalSettings.debugMode);

    CommandBufferInputChunk commandBufferInput = {logicalDevice, commandPool, swapchainFrames};
    makeFrameCommandBuffers(commandBufferInput);

}

void Vulkan::cleanupSwapchain() {
    for (const auto &frame: swapchainFrames) {
        logicalDevice.destroyImageView(frame.imageView);
        logicalDevice.destroyFramebuffer(frame.framebuffer);
        logicalDevice.destroyFence(frame.inFlight);
        logicalDevice.destroySemaphore(frame.imageAvailable);
        logicalDevice.destroySemaphore(frame.renderFinished);
    }

    logicalDevice.destroySwapchainKHR(swapchain);
}

void Vulkan::makePipeline() {
    GraphicsPipelineInBundle specification;
    specification.device = logicalDevice;
    specification.swapchainExtent = swapchainExtent;
    specification.swapchainImageFormat = swapchainFormat;
    specification.vertexFilepath = localPath + "Nest/res/Shaders/CompileShaders/vst.spv";
    specification.fragmentFilepath = localPath + "Nest/res/Shaders/CompileShaders/fst.spv";

    GraphicsPipelineOutBundle output = makeGraphicsPipeline(specification,
                                                            m_globalSettings.debugMode);
    pipeline = output.pipeline;
    pipelineLayout = output.layout;
    renderPass = output.renderPass;
}

void Vulkan::finalizeSetup() {
    makeFramebuffer();
    commandPool = makeCommandPool(logicalDevice, physicalDevice, surface, m_globalSettings.debugMode);

    CommandBufferInputChunk commandBufferInput = {logicalDevice, commandPool, swapchainFrames};

    mainCommandBuffer = makeCommandBuffer(commandBufferInput);
    makeFrameCommandBuffers(commandBufferInput);

    makeFrameSync();
}

void Vulkan::makeFramebuffer() {
    FramebufferInput framebufferInput;
    framebufferInput.device = logicalDevice;
    framebufferInput.renderPass = renderPass;
    framebufferInput.swapchainExtent = swapchainExtent;
    makeFrameBuffers(framebufferInput, swapchainFrames, m_globalSettings.debugMode);
}

void Vulkan::makeFrameSync() {
    for (auto &frame: swapchainFrames) {
        frame.inFlight = makeFence(logicalDevice, m_globalSettings.debugMode);
        frame.imageAvailable = makeSemaphore(logicalDevice, m_globalSettings.debugMode);
        frame.renderFinished = makeSemaphore(logicalDevice, m_globalSettings.debugMode);
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

    commandBuffer.draw(3, 1, 0, 0);

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

//     acquireNextImageKHR(SwapChainKHR, timeout, semaphore_to_signal, fence)

    uint32_t imageIndex;
    try {
        ResultValue acquire = logicalDevice.acquireNextImageKHR(
                swapchain, UINT64_MAX,
                swapchainFrames[frameNumber].imageAvailable, nullptr
        );
        imageIndex = acquire.value;
    } catch (const OutOfDateKHRError& error) {
        LOG_INFO("Recreate Swapchain");
        recreateSwapchain();
        return;
    } catch (const IncompatibleDisplayKHRError& error) {
        LOG_INFO("Recreate Swapchain");
        recreateSwapchain();
        return;
    } catch (const SystemError &error) {
        LOG_ERROR("Failed to acquire swapchain image!");
    }

    CommandBuffer commandBuffer = currentFrame.commandBuffer;

    commandBuffer.reset();

    recordDrawCommands(commandBuffer, imageIndex);

    SubmitInfo submitInfo;

    Semaphore waitSemaphores[] = {currentFrame.imageAvailable};
    PipelineStageFlags waitStages[] = {PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    Semaphore signalSemaphores[] = {currentFrame.renderFinished};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    logicalDevice.resetFences(1, &currentFrame.inFlight);
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

    SwapchainKHR swapChains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    Result present;
    try {
        present = presentQueue.presentKHR(presentInfo);
    } catch (const OutOfDateKHRError &error) {
        present = Result::eErrorOutOfDateKHR;
    }
    if (present == Result::eErrorOutOfDateKHR || present == Result::eSuboptimalKHR) {
        LOG_INFO("Recreate Swapchain");
        recreateSwapchain();
        return;
    }

    frameNumber = (frameNumber + 1) % maxFramesInFlight;
}
