#include "Nest/Renderer/Vulkan/Framebuffer.hpp"
#include "Nest/Logger/Logger.hpp"

using namespace vk;

void
FrameBufferInit::makeFrameBuffers(const FramebufferInput &inputChunk,
                                  std::vector<Swapchain::SwapChainFrame> &frames, bool debug) {
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
            if (debug) {
                LOG_INFO("Created framebuffer for frame {}", i);
            }
        } catch (const SystemError &err) {
            if (debug) {
                LOG_ERROR("Failed to create framebuffer for frame {}\n{}", i, err.what());
            }
        }

    }
}