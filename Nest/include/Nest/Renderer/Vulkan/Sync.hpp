#pragma once

#include <vulkan/vulkan.hpp>

using namespace vk;

// synchronization
Semaphore makeSemaphore(const Device &device, bool debug);

Fence makeFence(const Device &device, bool debug);