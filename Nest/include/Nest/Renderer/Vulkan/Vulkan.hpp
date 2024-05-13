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
private:
    void makeInstance();
    void makeDevice();

    GlobalSettings m_globalSettings;

    //instance-related variables
    Instance instance;
    DebugUtilsMessengerEXT debugMessenger;
    DispatchLoaderDynamic dld;
    SurfaceKHR surface;

    //device-related variables
    PhysicalDevice physicalDevice;
    Device logicalDevice;
    Queue graphicsQueue;
    Queue presentQueue;
    SwapchainKHR swapchain;
    std::vector<VulkanInit::SwapChainFrame> swapchainFrames;
    Format swapchainFormat;
    Extent2D swapchainExtent;
};