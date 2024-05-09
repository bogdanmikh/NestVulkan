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
    void init(bool debug, const char* appName) override;
private:
    void makeInstance(const char *appName);
    void makeDevice();

    bool debugMode;

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
};