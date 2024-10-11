/*
    Example for window with triangle in Vulkan with svklib
*/

#include "commandpool.hpp"
#include "shader.hpp"
#include "vkpipeline.hpp"
#include "vkrenderpass.hpp"
#include "window.hpp"
#include "log.hpp"
#include "vkswapchain.hpp"

#include <memory>
#include <stdexcept>

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

class App : public Window {
public:
    App() : Window("Window Example", {}) {
        Validation::enableValidationLayers = true;

        initVulkan({
            vk::KHRSurfaceExtensionName,
        },
#ifdef __MACH__
            true
#else
            false
#endif
        );
        device = requestDevice(
            vk::PhysicalDeviceFeatures(),
            {
#ifdef __MACH__
                "VK_KHR_portability_subset",
#endif
                vk::KHRSwapchainExtensionName,
            }
        );
        LOG_INFO("Chosen physical device {}", device->v_physical_device.getProperties(v_dispatcher).deviceName.data());

        swapchain = requestSwapchain(PreferredSwapchainSettings {
            .requestedCapabilities = vk::SurfaceCapabilitiesKHR(),
            .preferredFormat = vk::Format::eB8G8R8A8Srgb,
            .preferredPresentMode = vk::PresentModeKHR::eFifo
        });
        LOG_INFO("Swapchain created with extent {}x{}", 
            swapchain->v_swapchain_extent.width, swapchain->v_swapchain_extent.height
        );

        render_pass = std::make_unique<RenderPass>(
            *device,
            swapchain->v_image_format,
            vk::RenderPassCreateInfo(),
            v_dispatcher
        );

        Shader vertShader = Shader(
            *device,
            "shaders/triangle.vert.spv",
            vk::ShaderStageFlagBits::eVertex,
            v_dispatcher
        );
        Shader fragShader = Shader(
            *device,
            "shaders/triangle.frag.spv",
            vk::ShaderStageFlagBits::eFragment,
            v_dispatcher
        );
        std::vector<vk::PipelineShaderStageCreateInfo> shader_stages = {
            vertShader.v_stage_info, fragShader.v_stage_info
        };

        auto colorBlendInfo = vk::PipelineColorBlendStateCreateInfo()
            .setAttachments(vk::PipelineColorBlendAttachmentState()
                .setBlendEnable(vk::False)
            );
        
        auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
            .setPrimitiveRestartEnable(vk::False)
            .setTopology(vk::PrimitiveTopology::eTriangleList);
        
        auto vertexInputState = vk::PipelineVertexInputStateCreateInfo()
            .setVertexAttributeDescriptions({})
            .setVertexBindingDescriptions({});
        
        auto multisampleState = vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setSampleShadingEnable(vk::False);
        
        auto rasterizationState = vk::PipelineRasterizationStateCreateInfo()
            .setCullMode(vk::CullModeFlagBits::eNone)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setRasterizerDiscardEnable(vk::False)
            .setDepthClampEnable(vk::False)
            .setLineWidth(1.0)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setDepthBiasEnable(vk::False);
            
        auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
            .setPColorBlendState(&colorBlendInfo)
            .setPInputAssemblyState(&inputAssembly)
            .setPVertexInputState(&vertexInputState)
            .setPMultisampleState(&multisampleState)
            .setPRasterizationState(&rasterizationState);

        pipeline = std::make_unique<Pipeline>(
            *device,
            *render_pass,
            shader_stages,
            vk::PipelineLayoutCreateInfo(),
            pipelineInfo,
            v_dispatcher
        );

        swapchain->initFramebuffers(*render_pass);
        command_pool = std::make_unique<CommandPool>(
            *device,
            device->queue_family_indices.graphics,
            vk::CommandPoolCreateFlags(),
            v_dispatcher
        );

        graphicsCommandBuffer = command_pool->createCommandBuffer();
    }

    void recordCmdBuffer(uint32_t imageIndex) {
        graphicsCommandBuffer.begin(vk::CommandBufferBeginInfo());

        auto clearColor = vk::ClearValue(
            vk::ClearColorValue(1.0f, 1.0f, 1.0f, 1.0f)
        );
            
        auto renderPassBegin = vk::RenderPassBeginInfo()
            .setRenderPass(render_pass->v_render_pass)
            .setRenderArea({
                {0, 0},
                swapchain->v_swapchain_extent.height,
            }).setClearValues(clearColor)
            .setFramebuffer(swapchain->framebuffers[imageIndex]);
        
        graphicsCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline, v_dispatcher);

        graphicsCommandBuffer.bindPipeline(
            vk::PipelineBindPoint::eGraphics,
            pipeline->v_pipeline,
            v_dispatcher
        );

        auto viewport = vk::Viewport()
            .setWidth(swapchain->v_swapchain_extent.width)
            .setHeight(swapchain->v_swapchain_extent.height)
            .setMinDepth(0.0)
            .setMaxDepth(1.0)
            .setX(0.0)
            .setY(0.0);
        graphicsCommandBuffer.setViewport(0, viewport, v_dispatcher);
        
        auto scissor = vk::Rect2D()
            .setOffset({0, 0})
            .setExtent(swapchain->v_swapchain_extent);
        graphicsCommandBuffer.setScissor(0, scissor, v_dispatcher);

        graphicsCommandBuffer.draw(3, 1, 0, 0, v_dispatcher);

        graphicsCommandBuffer.endRenderPass(v_dispatcher);

        graphicsCommandBuffer.end(v_dispatcher);
    }

    void loop(double delta) {
        
    }

private:
    Device *device;
    Swapchain *swapchain;
    std::unique_ptr<RenderPass> render_pass;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<CommandPool> command_pool;

    vk::CommandBuffer graphicsCommandBuffer;
};

int main(void) {
    logging::set_log_level(LOGLEVEL_DEBUG);

    App *app;
    try {
        app = new App();
    } catch(std::runtime_error &error) {
        LOG_ERROR("Error occured while initializing application: {}", error.what());
        return 1;
    }

    while(!app->shouldClose()) {
        app->pollEvents();

        app->loop(0.0);
    }

    delete app;
}