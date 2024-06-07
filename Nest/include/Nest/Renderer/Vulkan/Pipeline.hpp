#pragma once

#include <vulkan/vulkan.hpp>

using namespace vk;

struct GraphicsPipelineInBundle {
    Device device;
    std::string vertexFilepath;
    std::string fragmentFilepath;
    Extent2D swapchainExtent;
    Format swapchainImageFormat;
};

struct GraphicsPipelineOutBundle {
    PipelineLayout layout;
    RenderPass renderPass;
    Pipeline pipeline;
};

GraphicsPipelineOutBundle makeGraphicsPipeline(const GraphicsPipelineInBundle &specification, bool debug);

PipelineLayout makePipelineLayout(const Device &device, bool debug);

RenderPass makeRenderpass(const Device &device, const Format &swapchainImageFormat, bool debug);