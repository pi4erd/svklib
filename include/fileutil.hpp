#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>

namespace utils {
    [[nodiscard]] std::string readFileToEnd(const std::string &path);
    [[nodiscard]] std::vector<uint8_t> readFileBinary(const std::string &path);
}

inline std::string utils::readFileToEnd(const std::string &path)
{
    std::ifstream file(path);

    if(!file.is_open()) {
        throw std::runtime_error("Failed to open file for reading!");
    }

    std::string result;

    std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(),
        std::back_inserter(result));

    return result;
}

inline std::vector<uint8_t> utils::readFileBinary(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);

    if(!file.is_open()) {
        throw std::runtime_error("Failed to open file for reading!");
    }

    std::vector<uint8_t> vec(std::istreambuf_iterator<char>(file), {});

    return vec;
}
