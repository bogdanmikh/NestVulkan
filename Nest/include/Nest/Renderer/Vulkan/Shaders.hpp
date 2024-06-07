#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan.hpp>

using namespace vk;

ShaderModule createModule(const std::string &filename, const Device &logicalDevice, bool debug);

std::vector<char> readFile(const std::string &filename, bool debug);