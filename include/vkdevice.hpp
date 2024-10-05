#pragma once

#include "validation.hpp"
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

struct QueueFamilyIndices {
    uint32_t graphics;

    QueueFamilyIndices(vk::PhysicalDevice &device, vk::DispatchLoaderDynamic &v_dispatcher) {
        vk::Optional<uint32_t> graphicsFamily = nullptr;

        auto properties = device.getQueueFamilyProperties(v_dispatcher);

        for(uint32_t i = 0; i < properties.size(); i++) {
            auto &family = properties[i];
            if(family.queueFlags & vk::QueueFlagBits::eGraphics) {
                graphicsFamily = i;
                break;
            }
        }

        if(!graphicsFamily) {
            throw std::runtime_error("Failed to find graphics queue family!");
        }

        graphics = *graphicsFamily;
    }
};

class Device {
public:
    Device(
        vk::Instance &instance,
        vk::PhysicalDeviceFeatures requestedFeatures,
        vk::DispatchLoaderDynamic &v_dispatcher
    ) : v_dispatcher(v_dispatcher){
        auto physical_devices = instance.enumeratePhysicalDevices(v_dispatcher);

        vk::Optional<vk::PhysicalDevice> chosenDevice = nullptr;

        for(auto &device : physical_devices) {
            auto features = device.getFeatures(v_dispatcher);

            // TODO: Implement device picking
            chosenDevice = device;
            break;
        }

        if(chosenDevice == nullptr) {
            throw std::runtime_error("Failed to find device with requested Vulkan features!");
        }

        v_physical_device = *chosenDevice;

        auto queueFamilies = QueueFamilyIndices(v_physical_device, v_dispatcher);

        std::vector<float> queuePriorities = {1.0f};

        auto queueInfo = vk::DeviceQueueCreateInfo()
            .setQueueFamilyIndex(queueFamilies.graphics)
            .setQueuePriorities(queuePriorities)
            .setQueueCount(1);

        

        auto deviceInfo = vk::DeviceCreateInfo()
            .setQueueCreateInfos({queueInfo})
            .setPEnabledFeatures(&requestedFeatures);
        
        if(Validation::enableValidationLayers) {
            deviceInfo = deviceInfo.setPEnabledLayerNames(Validation::validationLayers);
        }

        v_device = v_physical_device.createDevice(deviceInfo, nullptr, v_dispatcher);
    }
    ~Device() {
        v_device.destroy(nullptr, v_dispatcher);
    }

public:
    vk::Device v_device;
    vk::PhysicalDevice v_physical_device;
    vk::Queue v_queue;

    vk::DispatchLoaderDynamic &v_dispatcher;
};
