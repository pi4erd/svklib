#pragma once

#include <cstdint>
#include <iterator>
#include <string>
#include <fstream>

#include <vulkan/vulkan.hpp>

#include "vkdevice.hpp"

class Shader {
public:
    Shader(
        Device &device,
        const std::string &path,
        vk::ShaderStageFlagBits stage,
        vk::DispatchLoaderDynamic dispatcher,
        const std::string &entrypoint = "main"
    ) : device(device), v_dispatcher(dispatcher) {
        std::vector<uint8_t> code = readFile(path);

        auto shaderInfo = vk::ShaderModuleCreateInfo()
            .setPCode(reinterpret_cast<const uint32_t*>(code.data()))
            .setCodeSize(code.size());
        
        v_shader = device.v_device.createShaderModule(shaderInfo, nullptr, v_dispatcher);

        v_stage_info = vk::PipelineShaderStageCreateInfo()
            .setStage(stage)
            .setModule(v_shader);
    }

    ~Shader() {
        device.v_device.destroyShaderModule(v_shader, nullptr, v_dispatcher);
    }

public:
    Device &device;

    vk::ShaderModule v_shader;
    vk::PipelineShaderStageCreateInfo v_stage_info;
    vk::DispatchLoaderDynamic v_dispatcher;

private:
    [[nodiscard]] static const std::vector<uint8_t> readFile(const std::string &path) {
        std::ifstream file(path, std::ios::binary);

        std::vector<uint8_t> bytes(std::istreambuf_iterator<char>(), {});

        return bytes;
    }
};
