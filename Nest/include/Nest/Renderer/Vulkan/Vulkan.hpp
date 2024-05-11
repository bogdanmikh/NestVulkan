#pragma once

#include "Nest/Renderer/Renderer.hpp"

#include <vulkan/vulkan.hpp>
#include <vector>
#include <string>

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
    SwapchainKHR swapchain{ nullptr };
    std::vector<Image> swapchainImages{ nullptr };
    Format swapchainFormat;
    Extent2D swapchainExtent;
};