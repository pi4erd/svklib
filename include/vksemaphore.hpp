#pragma once

#include "vkdevice.hpp"
#include <vulkan/vulkan.hpp>

class Semaphore {
public:
    Semaphore(Device &device, vk::DispatchLoaderDynamic dispatcher) 
    : device(device), v_dispatcher(dispatcher) {
        v_semaphore = device.v_device.createSemaphore(vk::SemaphoreCreateInfo(), nullptr, v_dispatcher);
    }

    ~Semaphore() {
        device.v_device.destroySemaphore(v_semaphore, nullptr, v_dispatcher);
    }

public:
    Device &device;

    vk::Semaphore v_semaphore;
    vk::DispatchLoaderDynamic v_dispatcher;
};
