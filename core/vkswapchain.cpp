#include "vkswapchain.hpp"
#include "log.hpp"
#include "vkdevice.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <set>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>
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
) : device(device), v_surface(surface), framebuffer_render_pass(nullptr), v_dispatcher(dispatcher) {
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
    
    QueueFamilyIndices indices = device.queue_family_indices;

    if(indices.graphics != indices.present) {
        const std::vector<uint32_t> queueFamilyIndices = {indices.graphics, indices.present};
        swapchainInfo = swapchainInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueFamilyIndices);
    } else {
        swapchainInfo = swapchainInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    v_swapchain = device.v_device.createSwapchainKHR(swapchainInfo, nullptr, v_dispatcher);
    v_swapchain_extent = extent;
    v_format = chosenFormat;
    v_present_mode = chosenPresentMode;

    LOG_DEBUG("Created swapchain with extent {}x{}", extent.width, extent.height);

    images = device.v_device.getSwapchainImagesKHR(v_swapchain, v_dispatcher);

    imageViews = createImageViews();
    LOG_DEBUG("Created {} image views.", imageViews.size());
}

Swapchain::~Swapchain() {
    cleanupSwapchain();
}

void Swapchain::cleanupSwapchain() {
    if(framebuffers.size() != 0) {
        for(auto &framebuffer : framebuffers) {
            device.v_device.destroyFramebuffer(framebuffer, nullptr, v_dispatcher);
        }
    }
    
    for(auto &imageView : imageViews) {
        device.v_device.destroyImageView(imageView, nullptr, v_dispatcher);
    }

    device.v_device.destroySwapchainKHR(v_swapchain, nullptr, v_dispatcher);
    LOG_DEBUG("Destroyed swapchain.");
}


void Swapchain::recreate(int windowWidth, int windowHeight) {
    auto supportDetails = querySupportDetails(v_surface);
    bool recreateFramebuffers = framebuffers.size() != 0;

    auto extent = chooseExtent(windowWidth, windowHeight, supportDetails.capabilities);

    cleanupSwapchain();

    uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;
    if(supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount) {
        imageCount = supportDetails.capabilities.maxImageCount;
    }

    auto swapchainInfo = vk::SwapchainCreateInfoKHR()
        .setSurface(v_surface)
        .setMinImageCount(imageCount)
        .setImageFormat(v_format.format)
        .setImageColorSpace(v_format.colorSpace)
        .setPresentMode(v_present_mode)
        .setImageExtent(extent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setPreTransform(supportDetails.capabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setClipped(vk::True);
    
    QueueFamilyIndices indices = device.queue_family_indices;

    v_swapchain = device.v_device.createSwapchainKHR(swapchainInfo, nullptr, v_dispatcher);
    v_swapchain_extent = extent;

    LOG_DEBUG("Created swapchain with extent {}x{}", extent.width, extent.height);

    images = device.v_device.getSwapchainImagesKHR(v_swapchain, v_dispatcher);

    imageViews = createImageViews();
    LOG_DEBUG("Created {} image views.", imageViews.size());

    if(recreateFramebuffers) {
        initFramebuffers(*framebuffer_render_pass);
    }
}

void Swapchain::initFramebuffers(RenderPass &render_pass) {
    framebuffer_render_pass = render_pass;

    framebuffers.resize(imageViews.size());

    for(size_t i = 0; i < imageViews.size(); i++) {
        vk::ImageView &attachment = imageViews[i];

        auto framebufferInfo = vk::FramebufferCreateInfo()
            .setAttachments(attachment)
            .setWidth(v_swapchain_extent.width)
            .setHeight(v_swapchain_extent.height)
            .setRenderPass(render_pass.v_render_pass)
            .setLayers(1);
        
        framebuffers[i] = device.v_device.createFramebuffer(framebufferInfo, nullptr, v_dispatcher);
    }
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
            ).setFormat(v_format.format)
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

vk::ResultValue<uint32_t> Swapchain::acquireImage(
    vk::Optional<Semaphore> semaphore,
    vk::Optional<Fence> fence,
    uint64_t timeout
) {
    vk::Semaphore semaphoreChecked = semaphore == nullptr ? nullptr :
        semaphore->v_semaphore;
    vk::Fence fenceChecked = fence == nullptr ? nullptr :
        fence->v_fence;
    
    return device.v_device.acquireNextImageKHR(
        v_swapchain,
        timeout,
        semaphoreChecked,
        fenceChecked,
        v_dispatcher
    );
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
