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
        auto color_attachment = vk::AttachmentDescription()
            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
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
        auto subpass = vk::SubpassDescription()
            .setColorAttachments(ref);
        auto dep = vk::SubpassDependency();

        // Properties
        // Optional:
        // - Dependencies ???
        vk::RenderPassCreateInfo()
            .setSubpasses(subpass)
            .setAttachments(color_attachment);

        v_render_pass = device.v_device.createRenderPass(renderPassInfo);
    }

    ~RenderPass() {
        device.v_device.destroyRenderPass(v_render_pass, nullptr, v_dispatcher);
    }

public:
    Device &device;

    vk::RenderPass v_render_pass;
    vk::DispatchLoaderDynamic v_dispatcher;
};
