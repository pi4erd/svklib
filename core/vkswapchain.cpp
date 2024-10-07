#include "vkswapchain.hpp"
#include "log.hpp"
#include "vkdevice.hpp"

#include <algorithm>
#include <limits>
#include <set>

#include <vulkan/vulkan.hpp>
#ifndef __MACH__
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>
#endif

Swapchain::Swapchain(
    Device &device,
    int windowWidth, int windowHeight,
    vk::SurfaceKHR surface,
    PreferredSwapchainSettings preferredSettings,
    vk::DispatchLoaderDynamic &dispatcher
) : device(device), v_dispatcher(dispatcher) {
    auto supportDetails = querySupportDetails(surface);

    // std::set<vk::SurfaceFormatKHR> formatsUnique(supportDetails.formats.begin(), supportDetails.formats.end());
    std::set<vk::PresentModeKHR> presentModesUnique(supportDetails.presentModes.begin(), supportDetails.presentModes.end());

    vk::SurfaceFormatKHR chosenFormat;
    vk::PresentModeKHR chosenPresentMode;

    // TODO: Implement format matching
    chosenFormat = supportDetails.formats[0];

    // if(formatsUnique.find(preferredSettings.preferredFormat) != formatsUnique.end()) {
    //     chosenFormat = preferredSettings.preferredFormat;
    // } else {
    //     chosenFormat = supportDetails.formats[0];
    // }

    if(presentModesUnique.find(preferredSettings.preferredPresentMode) != presentModesUnique.end()) {
        chosenPresentMode = preferredSettings.preferredPresentMode;
    } else {
        chosenPresentMode = supportDetails.presentModes[0];
    }

    auto extent = chooseExtent(windowWidth, windowHeight, supportDetails.capabilities);
    
    uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;
    if(supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount) {
        imageCount = supportDetails.capabilities.maxImageCount;
    }

    auto swapchainInfo = vk::SwapchainCreateInfoKHR()
        .setSurface(surface)
        .setMinImageCount(imageCount)
        .setImageFormat(chosenFormat.format)
        .setImageColorSpace(chosenFormat.colorSpace)
        .setPresentMode(chosenPresentMode)
        .setImageExtent(extent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setPreTransform(supportDetails.capabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setClipped(vk::True);
    
    QueueFamilyIndices indices(device.v_physical_device, surface, v_dispatcher);
    if(indices.graphics != indices.present) {
        const std::vector<uint32_t> queueFamilyIndices = {indices.graphics, indices.present};
        swapchainInfo = swapchainInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueFamilyIndices);
    } else {
        swapchainInfo = swapchainInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    v_swapchain = device.v_device.createSwapchainKHR(swapchainInfo, nullptr, v_dispatcher);
    v_swapchain_extent = extent;
    v_image_format = chosenFormat.format;

    LOG_DEBUG("Created swapchain with extent {}x{}", extent.width, extent.height);

    images = device.v_device.getSwapchainImagesKHR(v_swapchain, v_dispatcher);

    imageViews = createImageViews();
    LOG_DEBUG("Created {} image views.", imageViews.size());
}

Swapchain::~Swapchain() {
    for(auto &imageView : imageViews) {
        device.v_device.destroyImageView(imageView, nullptr, v_dispatcher);
    }

    device.v_device.destroySwapchainKHR(v_swapchain, nullptr, v_dispatcher);
    LOG_DEBUG("Destroyed swapchain.");
}

std::vector<vk::ImageView> Swapchain::createImageViews() {
    std::vector<vk::ImageView> result;

    result.resize(images.size());
    for(size_t i = 0; i< images.size(); i++) {
        auto imageViewInfo = vk::ImageViewCreateInfo()
            .setImage(images[i])
            .setViewType(vk::ImageViewType::e2D)
            .setComponents(vk::ComponentMapping()
                .setA(vk::ComponentSwizzle::eIdentity)
                .setR(vk::ComponentSwizzle::eIdentity)
                .setG(vk::ComponentSwizzle::eIdentity)
                .setB(vk::ComponentSwizzle::eIdentity)
            ).setFormat(v_image_format)
            .setSubresourceRange(vk::ImageSubresourceRange()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1)
            );
        
        result[i] = device.v_device.createImageView(imageViewInfo, nullptr, v_dispatcher);
    }
    return result;
}


vk::Extent2D Swapchain::chooseExtent(int windowWidth, int windowHeight, vk::SurfaceCapabilitiesKHR &caps) {
    if(caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return caps.currentExtent;
    } else {
        vk::Extent2D extent = {
            static_cast<uint32_t>(windowWidth),
            static_cast<uint32_t>(windowHeight),
        };

        extent.width = std::clamp(extent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = std::clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);

        return extent;
    }
}

SwapChainSupportDetails Swapchain::querySupportDetails(vk::SurfaceKHR surface) {
    SwapChainSupportDetails details;

    auto caps = device.v_physical_device.getSurfaceCapabilitiesKHR(surface, v_dispatcher);
    auto formats = device.v_physical_device.getSurfaceFormatsKHR(surface, v_dispatcher);
    auto presentModes = device.v_physical_device.getSurfacePresentModesKHR(surface, v_dispatcher);

    details.capabilities = caps;
    details.formats = formats;
    details.presentModes = presentModes;

    return details;
}
