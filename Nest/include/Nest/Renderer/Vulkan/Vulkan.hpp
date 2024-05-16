#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <string>

#include "Nest/Renderer/Renderer.hpp"
#include "Swapchain.hpp"

using namespace vk;

class Vulkan final : public Renderer {
public:
    Vulkan();
    ~Vulkan() override;
    void init(const GlobalSettings &globalSettings) override;
    void render() override;
private:
    void makeInstance();
    void makeDevice();
    void makePipeline();
    void finalizeSetup();
    void recordDrawCommands(const CommandBuffer &commandBuffer, uint32_t imageIndex);

    GlobalSettings m_globalSettings;

    // Instance-related variables
    Instance instance;
    DebugUtilsMessengerEXT debugMessenger;
    DispatchLoaderDynamic dld;
    SurfaceKHR surface;

    // Device-related variables
    PhysicalDevice physicalDevice;
    Device logicalDevice;
    Queue graphicsQueue;
    Queue presentQueue;
    SwapchainKHR swapchain;
    std::vector<Swapchain::SwapChainFrame> swapchainFrames;
    Format swapchainFormat;
    Extent2D swapchainExtent;

    // Pipeline-related variables
    PipelineLayout pipelineLayout;
    RenderPass renderPass;
    Pipeline pipeline;

    // Command-related variables
    CommandPool commandPool; // responsible for memory allocation
    CommandBuffer mainCommandBuffer;

    // Synchronization objects
    Fence inFlightFence;
    Semaphore imageAvailable, rendererFinished;
};