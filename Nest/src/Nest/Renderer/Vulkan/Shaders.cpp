#include "Nest/Renderer/Vulkan/Shaders.hpp"
#include "Nest/Logger/Logger.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

std::vector<char> Shaders::readFile(const std::string &filename, bool debug) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (debug && !file.is_open()) {
        std::ostringstream stringStream;
        stringStream << "Failed to load \"" << filename << "\"";
        LOG_ERROR("{}", stringStream.str());
    }
    size_t filesize{ static_cast<size_t>(file.tellg()) };

    std::vector<char> buffer(filesize);
    file.seekg(0);
    file.read(buffer.data(), filesize);

    file.close();
    return buffer;
}

ShaderModule Shaders::createModule(const std::string &filename, const Device &logicalDevice, bool debug) {
    std::vector<char> sourceCode = readFile(filename, debug);
    ShaderModuleCreateInfo moduleInfo = {};
    moduleInfo.flags = ShaderModuleCreateFlags();
    moduleInfo.codeSize = sourceCode.size();
    moduleInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

    try {
        return logicalDevice.createShaderModule(moduleInfo);
    }
    catch (const SystemError &err) {
        if (debug) {
            std::ostringstream message;
            message << "Failed to create shader module for \"" << filename << "\"" << "\n" << err.what();
            LOG_ERROR("{}", message.str());
        }
    }
}
