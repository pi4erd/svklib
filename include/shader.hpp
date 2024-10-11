#pragma once

#include <cstdint>
#include <string>

#include <vulkan/vulkan.hpp>

#include "fileutil.hpp"
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
        std::vector<uint8_t> code = utils::readFileBinary(path);

        auto shaderInfo = vk::ShaderModuleCreateInfo()
            .setPCode(reinterpret_cast<const uint32_t*>(code.data()))
            .setCodeSize(code.size());
        
        v_shader = device.v_device.createShaderModule(shaderInfo, nullptr, v_dispatcher);

        v_stage_info = vk::PipelineShaderStageCreateInfo()
            .setStage(stage)
            .setModule(v_shader)
            .setPName(entrypoint.c_str());
    }

    ~Shader() {
        device.v_device.destroyShaderModule(v_shader, nullptr, v_dispatcher);
    }

public:
    Device &device;

    vk::ShaderModule v_shader;
    vk::PipelineShaderStageCreateInfo v_stage_info;
    vk::DispatchLoaderDynamic v_dispatcher;
};
