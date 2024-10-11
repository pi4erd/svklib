#pragma once

#include "log.hpp"
#include "shader.hpp"
#include "vkdevice.hpp"
#include "vkrenderpass.hpp"
#include <array>
#include <cassert>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

class Pipeline {
public:
    // TODO: Separate pipeline layout
    Pipeline(
        Device &device,
        RenderPass &render_pass,
        const std::vector<vk::PipelineShaderStageCreateInfo> &shader_stages,
        vk::PipelineLayoutCreateInfo layout_info,
        vk::GraphicsPipelineCreateInfo pipeline_info,
        vk::DispatchLoaderDynamic &dispatcher
    ): device(device), render_pass(render_pass), v_dispatcher(dispatcher) {
        std::array<vk::DynamicState, 2> dynamicStates = {
            vk::DynamicState::eScissor,
            vk::DynamicState::eViewport
        };

        auto dynamicStateInfo = vk::PipelineDynamicStateCreateInfo()
            .setDynamicStates(dynamicStates);

        v_layout = device.v_device.createPipelineLayout(layout_info);

        // Mandatory pipeline info:
        // - blend
        // - input assembly
        // - vertex state
        // - multisample state
        // - rasterization state
        // 
        // Optional
        // - depth stencil
        // - tesellation state
        pipeline_info = pipeline_info.setLayout(v_layout)
            .setRenderPass(render_pass.v_render_pass)
            .setSubpass(0)
            .setPViewportState(&vk::PipelineViewportStateCreateInfo()
                .setViewportCount(1)
                .setScissorCount(1)
            ).setStages(shader_stages)
            .setPDynamicState(&dynamicStateInfo);

        auto result = device.v_device.createGraphicsPipeline(nullptr, pipeline_info, nullptr, v_dispatcher);

        if(result.result != vk::Result::eSuccess && result.result != vk::Result::ePipelineCompileRequiredEXT) {
            throw std::runtime_error(
                fmt::format("Failed to create graphics pipeline: code {}.", (uint32_t)result.result)
            );
        }

        v_pipeline = result.value;
        LOG_DEBUG("Created GraphicsPipeline.");
    }

    Pipeline(
        Device &device,
        RenderPass &render_pass,
        const std::vector<Shader> &shader_stages,
        vk::PipelineLayoutCreateInfo layout_info,
        vk::ComputePipelineCreateInfo info,
        vk::DispatchLoaderDynamic &dispatcher
    ): device(device), render_pass(render_pass), v_dispatcher(dispatcher) {
        assert(0 && "Not yet implemented.");
    }

    ~Pipeline() {
        device.v_device.destroyPipelineLayout(v_layout, nullptr, v_dispatcher);
        device.v_device.destroyPipeline(v_pipeline, nullptr, v_dispatcher);

        LOG_DEBUG("Destroy Pipeline");
    }

public:
    Device &device;
    RenderPass &render_pass;

    vk::PipelineLayout v_layout;
    vk::Pipeline v_pipeline;
    vk::DispatchLoaderDynamic &v_dispatcher;
};
