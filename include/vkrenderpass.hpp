#pragma once

#include "vkdevice.hpp"
#include <vulkan/vulkan.hpp>

class RenderPass {
public:
    RenderPass(
        Device &device,
        vk::Format format,
        vk::RenderPassCreateInfo renderPassInfo,
        vk::DispatchLoaderDynamic dispatcher
    ) : device(device), v_dispatcher(dispatcher) {
        // Default color attachment at 0
        auto color_attachment = vk::AttachmentDescription()
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFormat(format)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setSamples(vk::SampleCountFlagBits::e1);
        auto ref = vk::AttachmentReference()
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
        
        // Subpass for color attachment
        auto subpass = vk::SubpassDescription()
            .setColorAttachments(ref)
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
        
        // Dependency for color attachment
        auto dep = vk::SubpassDependency()
            .setSrcSubpass(vk::SubpassExternal)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits())
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

        // TODO: Figure out a way to expose API without ruining defaults
        // Properties
        // Optional:
        // - Dependencies
        renderPassInfo = renderPassInfo
            .setSubpasses(subpass)
            .setAttachments(color_attachment)
            .setDependencies(dep);

        v_render_pass = device.v_device.createRenderPass(renderPassInfo);
    }

    ~RenderPass() {
        device.v_device.destroyRenderPass(v_render_pass, nullptr, v_dispatcher);
    }

    vk::RenderPass operator*() {
        return v_render_pass;
    }

public:
    Device &device;

    vk::RenderPass v_render_pass;
    vk::DispatchLoaderDynamic v_dispatcher;
};
