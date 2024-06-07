#include "Nest/Renderer/Vulkan/Framebuffer.hpp"
#include "Nest/Logger/Logger.hpp"
#include "Nest/Settings/SettingsLog.hpp"

using namespace vk;

void
makeFrameBuffers(const FramebufferInput &inputChunk,
                 std::vector<SwapChainFrame> &frames, bool debug) {
    for (int i = 0; i < frames.size(); ++i) {
        std::vector<ImageView> attachments = {
                frames[i].imageView
        };

        FramebufferCreateInfo framebufferInfo;
        framebufferInfo.flags = FramebufferCreateFlags();
        framebufferInfo.renderPass = inputChunk.renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = inputChunk.swapchainExtent.width;
        framebufferInfo.height = inputChunk.swapchainExtent.height;
        framebufferInfo.layers = 1;

        try {
            frames[i].framebuffer = inputChunk.device.createFramebuffer(framebufferInfo);
            if (VK_PRINT_GRAPTHICS_PIPELINE_INFO) {
                LOG_INFO("Created framebuffer for frame {}", i);
            }
        } catch (const SystemError &err) {
            if (VK_PRINT_GRAPTHICS_PIPELINE_INFO) {
                LOG_ERROR("Failed to create framebuffer for frame {}\n{}", i, err.what());
            }
        }

    }
}