#pragma once

#include <vulkan/vulkan.hpp>

#include "vkdevice.hpp"

class CommandPool {
public:
    CommandPool(
        Device &device,
        uint32_t queue_family_index,
        vk::CommandPoolCreateFlags flags,
        vk::DispatchLoaderDynamic dispatcher
    ) : device(device), v_dispatcher(dispatcher) {
        auto commandPoolInfo = vk::CommandPoolCreateInfo()
            .setFlags(flags)
            .setQueueFamilyIndex(queue_family_index);
            
        v_command_pool = device.v_device.createCommandPool(commandPoolInfo, nullptr, v_dispatcher);
    }

    ~CommandPool() {
        device.v_device.destroyCommandPool(v_command_pool, nullptr, v_dispatcher);
    }

    vk::CommandPool operator*() {
        return v_command_pool;
    }

    std::vector<vk::CommandBuffer> createCommandBuffers(
        uint32_t n,
        vk::CommandBufferLevel level=vk::CommandBufferLevel::ePrimary
    ) {
        auto allocateInfo = vk::CommandBufferAllocateInfo()
            .setCommandBufferCount(n)
            .setCommandPool(v_command_pool)
            .setLevel(level);
        
        std::vector<vk::CommandBuffer> vkCmdBuffers = device.v_device.allocateCommandBuffers(
            allocateInfo,
            v_dispatcher
        );

        // command_buffers.insert(command_buffers.end(), cmdBuffer.begin(), cmdBuffer.end());

        return vkCmdBuffers;
    }
    
    vk::CommandBuffer createCommandBuffer(vk::CommandBufferLevel level=vk::CommandBufferLevel::ePrimary) {
        return createCommandBuffers(1, level)[0];
    }

public:
    Device &device;

    // Might not be necessary
    // std::vector<vk::CommandBuffer> command_buffers;

    vk::CommandPool v_command_pool;
    vk::DispatchLoaderDynamic v_dispatcher;
};
