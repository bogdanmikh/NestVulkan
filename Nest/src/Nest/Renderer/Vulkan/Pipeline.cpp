#include "Nest/Renderer/Vulkan/Pipeline.hpp"
#include "Nest/Renderer/Vulkan/Shaders.hpp"
#include "Nest/Logger/Logger.hpp"

#include <vector>
#include <sstream>

PipelineLayout PipelineInit::makePipelineLayout(const Device &device, bool debug) {
    PipelineLayoutCreateInfo layoutInfo;
    layoutInfo.flags = PipelineLayoutCreateFlags();
    layoutInfo.setLayoutCount = 0;
    layoutInfo.pushConstantRangeCount = 0;
    try {
        return device.createPipelineLayout(layoutInfo);
    }
    catch (const SystemError &err) {
        if (debug) {
            LOG_ERROR("Failed to create pipeline layout!\n{}", err.what());
        }
    }
}

RenderPass PipelineInit::makeRenderpass(const Device &device, const Format &swapchainImageFormat, bool debug) {
    // Define a general attachment, with its load/store operations
    AttachmentDescription colorAttachment;
    colorAttachment.flags = AttachmentDescriptionFlags();
    colorAttachment.format = swapchainImageFormat;
    colorAttachment.samples = SampleCountFlagBits::e1;
    colorAttachment.loadOp = AttachmentLoadOp::eClear;
    colorAttachment.storeOp = AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = ImageLayout::eUndefined;
    colorAttachment.finalLayout = ImageLayout::ePresentSrcKHR;

    // Declare that attachment to be color buffer 0 of the framebuffer
    AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = ImageLayout::eColorAttachmentOptimal;

    // Renderpasses are broken down into subpasses, there's always at least one.
    SubpassDescription subpass;
    subpass.flags = SubpassDescriptionFlags();
    subpass.pipelineBindPoint = PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Now create the renderpass
    RenderPassCreateInfo renderpassInfo;
    renderpassInfo.flags = RenderPassCreateFlags();
    renderpassInfo.attachmentCount = 1;
    renderpassInfo.pAttachments = &colorAttachment;
    renderpassInfo.subpassCount = 1;
    renderpassInfo.pSubpasses = &subpass;
    try {
        return device.createRenderPass(renderpassInfo);
    } catch (const SystemError &err) {
        if (debug) {
            LOG_ERROR("Failed to create renderpass!\n{}", err.what());
        }
    }
}

PipelineInit::GraphicsPipelineOutBundle
PipelineInit::makeGraphicsPipeline(const PipelineInit::GraphicsPipelineInBundle &specification, bool debug) {
    // The info for the graphics pipeline
    GraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.flags = PipelineCreateFlags();

    // Shader stages, to be populated later
    std::vector <PipelineShaderStageCreateInfo> shaderStages;

    // Vertex Input
    PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.flags = PipelineVertexInputStateCreateFlags();
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;

    //Input Assembly
    PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo.flags = PipelineInputAssemblyStateCreateFlags();
    inputAssemblyInfo.topology = PrimitiveTopology::eTriangleList;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;

    // Vertex Shader
    if (debug) {
        LOG_INFO("Create vertex shader module");
    }
    ShaderModule vertexShader = Shaders::createModule(
            specification.vertexFilepath, specification.device, debug
    );
    PipelineShaderStageCreateInfo vertexShaderInfo;
    vertexShaderInfo.flags = PipelineShaderStageCreateFlags();
    vertexShaderInfo.stage = ShaderStageFlagBits::eVertex;
    vertexShaderInfo.module = vertexShader;
    vertexShaderInfo.pName = "main";
    shaderStages.push_back(vertexShaderInfo);

    // Viewport and Scissor
    Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) specification.swapchainExtent.width;
    viewport.height = (float) specification.swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    Rect2D scissor;
    scissor.offset.x = 0.0f;
    scissor.offset.y = 0.0f;
    scissor.extent = specification.swapchainExtent;
    PipelineViewportStateCreateInfo viewportState;
    viewportState.flags = PipelineViewportStateCreateFlags();
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    pipelineCreateInfo.pViewportState = &viewportState;

    // Rasterizer
    PipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.flags = PipelineRasterizationStateCreateFlags();
    rasterizer.depthClampEnable = VK_FALSE; // discard out of bounds fragments, don't clamp them
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // This flag would disable fragment output
    rasterizer.polygonMode = PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = CullModeFlagBits::eBack; // back not renderer
    rasterizer.frontFace = FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE; // Depth bias can be useful in shadow maps.
    pipelineCreateInfo.pRasterizationState = &rasterizer;

    // Fragment Shader
    if (debug) {
        LOG_INFO("Create fragment shader module");
    }
    ShaderModule fragmentShader = Shaders::createModule(
            specification.fragmentFilepath, specification.device, debug
    );
    PipelineShaderStageCreateInfo fragmentShaderInfo;
    fragmentShaderInfo.flags = PipelineShaderStageCreateFlags();
    fragmentShaderInfo.stage = ShaderStageFlagBits::eFragment;
    fragmentShaderInfo.module = fragmentShader;
    fragmentShaderInfo.pName = "main";
    shaderStages.emplace_back(fragmentShaderInfo);
    // Now both shaders have been made, we can declare them to the pipeline info
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();


    // Multisampling
    PipelineMultisampleStateCreateInfo multisampling;
    multisampling.flags = PipelineMultisampleStateCreateFlags();
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = SampleCountFlagBits::e1;
    pipelineCreateInfo.pMultisampleState = &multisampling;

    // Color Blend
    PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask =
            ColorComponentFlagBits::eR | ColorComponentFlagBits::eG | ColorComponentFlagBits::eB |
            ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;
    PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.flags = PipelineColorBlendStateCreateFlags();
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    pipelineCreateInfo.pColorBlendState = &colorBlending;

    // Pipeline Layout(can set uniform, textures)
    if (debug) {
        LOG_INFO("Create Pipeline Layout");
    }
    PipelineLayout pipelineLayout = makePipelineLayout(specification.device, debug);
    pipelineCreateInfo.layout = pipelineLayout;
    // RenderPass
    if (debug) {
        LOG_INFO("Create RenderPass");
    }

    RenderPass renderPass = makeRenderpass(specification.device, specification.swapchainImageFormat, debug);
    pipelineCreateInfo.renderPass = renderPass;

    // Extra stuff
    pipelineCreateInfo.basePipelineHandle = nullptr;

    // Make the Pipeline
    if (debug) {
        LOG_INFO("Create Graphics Pipeline");
    }
    Pipeline graphicsPipeline;

    try {
        graphicsPipeline = (specification.device.createGraphicsPipeline(nullptr, pipelineCreateInfo)).value;
    } catch (const SystemError &err) {
        if (debug) {
            LOG_ERROR("Failed to create Pipeline");
        }
    }

    PipelineInit::GraphicsPipelineOutBundle output;
    output.layout = pipelineLayout;
    output.renderPass = renderPass;
    output.pipeline = graphicsPipeline;

    specification.device.destroyShaderModule(vertexShader);
    specification.device.destroyShaderModule(fragmentShader);

    return output;
}