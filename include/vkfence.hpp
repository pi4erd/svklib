#pragma once

#include "vkdevice.hpp"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

class Fence {
public:
    Fence(Device &device, bool signaled, vk::DispatchLoaderDynamic dispatcher) 
    : device(device), v_dispatcher(dispatcher) {
        v_fence = device.v_device.createFence(
            vk::FenceCreateInfo().setFlags(
                signaled ? vk::FenceCreateFlagBits::eSignaled :
                vk::FenceCreateFlagBits()
            ),
            nullptr,
            v_dispatcher
        );
    }

    ~Fence() {
        device.v_device.destroyFence(v_fence, nullptr, v_dispatcher);
    }

public:
    Device &device;

    vk::Fence v_fence;
    vk::DispatchLoaderDynamic v_dispatcher;
};