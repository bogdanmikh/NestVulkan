#include "Nest/Renderer/Vulkan/Commands.hpp"
#include "Nest/Renderer/Vulkan/QueueFamilies.hpp"
#include "Nest/Logger/Logger.hpp"
#include "Nest/Settings/SettingsLog.hpp"

CommandPool
makeCommandPool(const Device &device, const PhysicalDevice &physicalDevice, const SurfaceKHR &surface,
                bool debug) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

    CommandPoolCreateInfo poolInfo;
    poolInfo.flags = CommandPoolCreateFlags() | CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    try {
        return device.createCommandPool(poolInfo);
    } catch (const SystemError &err) {
        if (debug) {
            LOG_ERROR("Failed to create Command Pool\n{}", err.what());
        }
        return nullptr;
    }
}

CommandBuffer makeCommandBuffer(const CommandBufferInputChunk &inputChunk) {
    CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = inputChunk.commandPool;
    allocInfo.level = CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    bool debug = VK_PRINT_COMMANDBUFFER_INFO;

    // Make a "main" command buffer for the engine
    try {
        CommandBuffer commandBuffer = inputChunk.device.allocateCommandBuffers(allocInfo)[0];
        if (debug) {
            LOG_INFO("Allocated main command buffer");
        }
        return commandBuffer;
    } catch (const SystemError &err) {
        if (debug) {
            LOG_ERROR("Failed to allocate main command buffer\n{}", err.what());
        }
        return nullptr;
    }
}

void makeFrameCommandBuffers(const CommandBufferInputChunk &inputChunk) {
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = inputChunk.commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    bool debug = VK_PRINT_COMMANDBUFFER_INFO;
    // make a command buffer for each frame
    for (int i = 0; i < inputChunk.frames.size(); ++i) {
        try {
            inputChunk.frames[i].commandBuffer = inputChunk.device.allocateCommandBuffers(allocInfo)[0];
            if (debug) {
                LOG_INFO("Allocated command buffer for frame {}", i);
            }
        } catch (const SystemError &err) {
            if (debug) {
                LOG_INFO("Failed to allocate command buffer for frame {}", i);
            }
        }
    }
}