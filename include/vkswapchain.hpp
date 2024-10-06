#pragma once

#include "vkdevice.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct PreferredSwapchainSettings {
    vk::SurfaceCapabilitiesKHR requestedCapabilities;
    vk::Format preferredFormat;
    vk::PresentModeKHR preferredPresentMode;
};

class Swapchain {
public:
    Swapchain(
        Device &device,
        int windowWidth, int windowHeight,
        vk::SurfaceKHR surface,
        PreferredSwapchainSettings preferredSettings,
        vk::DispatchLoaderDynamic &dispatcher
    );
    ~Swapchain();

    std::vector<vk::ImageView> createImageViews();

    SwapChainSupportDetails querySupportDetails(vk::SurfaceKHR surface);

private:
    vk::Extent2D chooseExtent(int windowWidth, int windowHeight, vk::SurfaceCapabilitiesKHR &caps);

public:
    Device &device;

    std::vector<vk::Image> images;
    std::vector<vk::ImageView> imageViews;

    vk::Format v_image_format;
    vk::Extent2D v_swapchain_extent;

    vk::DispatchLoaderDynamic &v_dispatcher;
    vk::SwapchainKHR v_swapchain;
};
