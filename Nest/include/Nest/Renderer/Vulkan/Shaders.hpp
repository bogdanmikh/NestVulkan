#pragma once

#include <vector>
#include <string>

class Shaders {
public:
    static std::vector<char> readFile(const std::string &filename, bool debug);
};