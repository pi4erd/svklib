#pragma once

#include "log.hpp"
#include "vkdevice.hpp"
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

class Buffer;

class MemoryMap {
public:
    MemoryMap(
        Device &device,
        vk::DeviceMemory device_memory,
        vk::DeviceAddress memory_size,
        vk::DispatchLoaderDynamic &dispatcher,
        vk::MemoryMapFlags flags=vk::MemoryMapFlags()
    ): device(device), v_device_memory(device_memory), v_dispatcher(dispatcher) {
        mappedMemory = device->mapMemory(
            v_device_memory,
            0,
            memory_size,
            flags,
            v_dispatcher
        );
    }

    ~MemoryMap() {
        device->unmapMemory(v_device_memory, v_dispatcher);
    }

    void *operator*() {
        return mappedMemory;
    }

public:
    Device &device;

    vk::DeviceMemory v_device_memory;

    void *mappedMemory;

    vk::DispatchLoaderDynamic &v_dispatcher;
};

class Buffer {
public:
    Buffer(
        Device &device,
        vk::BufferCreateInfo &buffer_info,
        vk::MemoryPropertyFlags memory_properties,
        vk::DispatchLoaderDynamic &dispatcher
    ): device(device), v_buffer_size(buffer_info.size), v_dispatcher(dispatcher) {
        v_buffer = device->createBuffer(buffer_info, nullptr, v_dispatcher);

        auto memoryReqs = device->getBufferMemoryRequirements(v_buffer, v_dispatcher);

        auto allocateInfo = vk::MemoryAllocateInfo()
            .setAllocationSize(memoryReqs.size)
            .setMemoryTypeIndex(findMemoryType(memoryReqs.memoryTypeBits, memory_properties));

        v_memory = device->allocateMemory(allocateInfo, nullptr, v_dispatcher);
        device->bindBufferMemory(v_buffer, v_memory, 0);
    }

    ~Buffer() {
        device->destroyBuffer(v_buffer, nullptr, v_dispatcher);
        device->freeMemory(v_memory, nullptr, v_dispatcher);
    }

    MemoryMap mapMemory() {
        return MemoryMap(device, v_memory, v_buffer_size, v_dispatcher);
    }

    vk::Buffer operator*() {
        return v_buffer;
    }

    vk::Buffer *operator->() {
        return &v_buffer;
    }

private:
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
        auto memProperties = device.v_physical_device.getMemoryProperties(v_dispatcher);

        for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if((typeFilter & (1 << i)) && 
               (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
            {
                return i;
            }
        }

        THROW(runtime_error, "Failed to find suitable memory type for a buffer.");
    }

public:
    Device &device;

    vk::Buffer v_buffer;

    // TODO: Separate device memory and buffer (buffer can have multiple memories attached)
    vk::DeviceMemory v_memory;
    vk::DeviceAddress v_buffer_size;

    vk::DispatchLoaderDynamic &v_dispatcher;
};
