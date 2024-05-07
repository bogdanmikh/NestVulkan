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
    static bool supported(std::vector<const char*> &needExtensions, std::vector<const char*> &layers, bool debug);
    void makeInstance(bool debug, const char *appName);
    void makeDebugMessenger();

    void makeDevice(bool debug);

    // vulkan instance
    Instance instance;
    // debug messenger
    DebugUtilsMessengerEXT debugMessenger;
    // dynamic instance dispatcher
    DispatchLoaderDynamic dld;
    // physical device(GPU)
    PhysicalDevice physicalDevice;
    // abstract physical device
    Device logicalDevice;
    // queue
    Queue graphicsQueue;
};