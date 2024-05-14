#include "Nest/Renderer/Vulkan/Shaders.hpp"
#include "Nest/Logger/Logger.hpp"

#include <fstream>
#include <iostream>

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