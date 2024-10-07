#pragma once

#include "log.hpp"
#include "validation.hpp"
#include <optional>
#include <set>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#ifndef __MACH__
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>
#endif

struct QueueFamilyIndices {
    uint32_t graphics;
    uint32_t present;

    QueueFamilyIndices(vk::PhysicalDevice &device, vk::SurfaceKHR &surface, vk::DispatchLoaderDynamic &v_dispatcher) {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        auto properties = device.getQueueFamilyProperties(v_dispatcher);

        vk::Bool32 presentSupport = vk::False;

        for(uint32_t i = 0; i < properties.size(); i++) {
            auto &family = properties[i];
            if(!graphicsFamily.has_value() && family.queueFlags & vk::QueueFlagBits::eGraphics) {
                graphicsFamily = i;
            }

            presentSupport = device.getSurfaceSupportKHR(i, surface, v_dispatcher);
            if(!presentFamily.has_value() && presentSupport) {
                presentFamily = i;
            }

            if(graphicsFamily.has_value() && presentFamily.has_value())
                break;
        }

        if(!graphicsFamily.has_value()) {
            throw std::runtime_error("Failed to find graphics queue family!");
        }

        if(!presentFamily.has_value()) {
            throw std::runtime_error("Failed to find present queue family!");
        }

        graphics = *graphicsFamily;
        present = *presentFamily;
    }
};

class Device {
public:
    Device(
        vk::Instance &instance,
        vk::SurfaceKHR &surface,
        vk::PhysicalDeviceFeatures requestedFeatures,
        const std::vector<const char*> &requestedExtensions,
        vk::DispatchLoaderDynamic &v_dispatcher
    ) : v_dispatcher(v_dispatcher){
        auto physical_devices = instance.enumeratePhysicalDevices(v_dispatcher);

        vk::Optional<vk::PhysicalDevice> chosenDevice = nullptr;

        for(auto &device : physical_devices) {
            auto features = device.getFeatures(v_dispatcher);

            // TODO: Implement feature check

            std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());

            for(auto &availableExtension : 
                device.enumerateDeviceExtensionProperties(nullptr, v_dispatcher))
            {
                requiredExtensions.erase(availableExtension.extensionName);
            }

            if(requiredExtensions.empty()) {
                chosenDevice = device;
                break;
            }
        }

        if(chosenDevice == nullptr) {
            throw std::runtime_error("Failed to find device with requested Vulkan features!");
        }

        v_physical_device = *chosenDevice;

        auto queueFamilies = QueueFamilyIndices(v_physical_device, surface, v_dispatcher);

        std::set<uint32_t> uniqueQueueFamilies = {
            queueFamilies.graphics,
            queueFamilies.present,
        };
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

        for(auto queueFamily : uniqueQueueFamilies) {
            const std::vector<float> queuePriorities = {1.0f};
            auto queueInfo = vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(queueFamily)
                .setQueuePriorities(queuePriorities)
                .setQueueCount(1);
            queueCreateInfos.push_back(queueInfo);
        }

        auto deviceInfo = vk::DeviceCreateInfo()
            .setQueueCreateInfos(queueCreateInfos)
            .setPEnabledExtensionNames(requestedExtensions)
            .setPEnabledFeatures(&requestedFeatures);
        
        if(Validation::enableValidationLayers) {
            deviceInfo = deviceInfo.setPEnabledLayerNames(Validation::validationLayers);
        }

        v_device = v_physical_device.createDevice(deviceInfo, nullptr, v_dispatcher);
        LOG_DEBUG("Created Vulkan device for {}.", v_physical_device.getProperties(v_dispatcher).deviceName.data());

        v_dispatcher.init(v_device);

        v_queue = v_device.getQueue(queueFamilies.graphics, 0, v_dispatcher);
        LOG_DEBUG("Created graphics queue.");

        v_present_queue = v_device.getQueue(queueFamilies.present, 0, v_dispatcher);
        LOG_DEBUG("Created present queue.");
    }

    ~Device() {
        LOG_DEBUG("Destroyed Vulkan device for {}.", v_physical_device.getProperties(v_dispatcher).deviceName.data());
        v_device.destroy(nullptr, v_dispatcher);
    }

public:
    vk::Device v_device;
    vk::PhysicalDevice v_physical_device;

    vk::Queue v_queue;
    vk::Queue v_present_queue;

    vk::DispatchLoaderDynamic &v_dispatcher;
};
