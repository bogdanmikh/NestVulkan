#include "Nest/Renderer/Vulkan/Sync.hpp"
#include "Nest/Logger/Logger.hpp"

Semaphore makeSemaphore(const Device &device, bool debug) {
    SemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.flags = SemaphoreCreateFlags();

    try {
        return device.createSemaphore(semaphoreInfo);
    } catch (const SystemError &err) {
        if (debug) {
            LOG_ERROR("Failed to create semaphore\n{}", err.what());
        }
        return nullptr;
    }
}

Fence makeFence(const Device &device, bool debug) {
    FenceCreateInfo fenceInfo;
    fenceInfo.flags = FenceCreateFlags() | FenceCreateFlagBits::eSignaled;

    try {
        return device.createFence(fenceInfo);
    } catch (const SystemError &err) {
        if (debug) {
            LOG_ERROR("Failed to create fence\n{}", err.what());
        }
        return nullptr;
    }
}