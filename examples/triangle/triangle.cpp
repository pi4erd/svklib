/*
    Example for window with triangle in Vulkan with svklib
*/

#include "commandpool.hpp"
#include "shader.hpp"
#include "vkfence.hpp"
#include "vkpipeline.hpp"
#include "vkrenderpass.hpp"
#include "vksemaphore.hpp"
#include "window.hpp"
#include "log.hpp"
#include "vkswapchain.hpp"

#include <limits>
#include <memory>
#include <stdexcept>

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>
#include <vulkan/vulkan_to_string.hpp>

class App : public Window {
public:
    App() : Window("Triangle Example", {}) {
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
            swapchain->v_format.format,
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
                .setColorWriteMask(vk::ColorComponentFlagBits::eR |
                    vk::ColorComponentFlagBits::eG |
                    vk::ColorComponentFlagBits::eB |
                    vk::ColorComponentFlagBits::eA
                )
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
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            v_dispatcher
        );

        graphicsCommandBuffer = command_pool->createCommandBuffer();

        image_ready = std::make_unique<Semaphore>(*device, v_dispatcher);
        render_finished = std::make_unique<Semaphore>(*device, v_dispatcher);
        in_flight_fence = std::make_unique<Fence>(*device, true, v_dispatcher);
    }

    ~App() {
        device->v_device.waitIdle(v_dispatcher);
    }

    void recordCmdBuffer(uint32_t imageIndex) {
        graphicsCommandBuffer.begin(vk::CommandBufferBeginInfo());

        auto clearColor = vk::ClearValue(
            vk::ClearColorValue(0.1f, 0.2f, 0.3f, 1.0f)
        );
        
        vk::Rect2D renderArea = {
            {0, 0},
            swapchain->v_swapchain_extent,
        };

        auto renderPassBegin = vk::RenderPassBeginInfo()
            .setRenderPass(render_pass->v_render_pass)
            .setRenderArea(renderArea)
            .setClearValues(clearColor)
            .setFramebuffer(swapchain->framebuffers[imageIndex]);
        
        graphicsCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline, v_dispatcher);

        graphicsCommandBuffer.bindPipeline(
            vk::PipelineBindPoint::eGraphics,
            pipeline->v_pipeline,
            v_dispatcher
        );

        auto viewport = vk::Viewport()
            .setWidth(static_cast<float>(swapchain->v_swapchain_extent.width))
            .setHeight(static_cast<float>(swapchain->v_swapchain_extent.height))
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
        vk::Result waitResult = device->v_device.waitForFences(
            in_flight_fence->v_fence,
            vk::True,
            std::numeric_limits<uint64_t>::max(),
            v_dispatcher
        );
        if(waitResult != vk::Result::eSuccess) {
            THROW(runtime_error, "Failed to wait on fences: {}.", vk::to_string(waitResult));
        }

        auto acquireResult = swapchain->acquireImage(*image_ready, nullptr);
        switch((uint32_t)acquireResult.result) {
            case (uint32_t)vk::Result::eSuboptimalKHR:
            case (uint32_t)vk::Result::eSuccess:
            device->v_device.resetFences(in_flight_fence->v_fence, v_dispatcher);
            break;
            case (uint32_t)vk::Result::eErrorOutOfDateKHR:
            if(width == 0 || height == 0) {
                glfwGetFramebufferSize(getWindow(), &width, &height);
                glfwWaitEvents();
            }
            swapchain->recreate(width, height);
            return;
            default:
            THROW(runtime_error, "Failed to acquire image: {}.", vk::to_string(acquireResult.result));
        }
        
        uint32_t imageIndex = acquireResult.value;

        graphicsCommandBuffer.reset();
        recordCmdBuffer(imageIndex);

        std::vector<vk::PipelineStageFlags> waitStages = {
            vk::PipelineStageFlagBits::eColorAttachmentOutput
        };
        
        auto renderSubmit = vk::SubmitInfo()
            .setWaitSemaphores(image_ready->v_semaphore)
            .setWaitDstStageMask(waitStages)
            .setCommandBuffers(graphicsCommandBuffer)
            .setSignalSemaphores(render_finished->v_semaphore);

        // LOG_DEBUG("Submitting rendering frame {}", frame);
        device->v_queue.submit({renderSubmit}, in_flight_fence->v_fence, v_dispatcher);

        auto presentInfo = vk::PresentInfoKHR()
            .setImageIndices(imageIndex)
            .setSwapchains(swapchain->v_swapchain)
            .setWaitSemaphores(render_finished->v_semaphore);
        
        // LOG_DEBUG("Presenting frame {}", frame);
        auto presentResult = device->v_present_queue.presentKHR(presentInfo, v_dispatcher);

        switch((uint64_t)presentResult) {
            case (uint64_t)vk::Result::eSuccess:
            break;
            case (uint64_t)vk::Result::eSuboptimalKHR:
            case (uint64_t)vk::Result::eErrorOutOfDateKHR:
            swapchain->recreate(width, height);
            break;
            default:
            THROW(runtime_error, fmt::format("Failed to present: {}", vk::to_string(presentResult)));
        }

        frame++;
    }

protected:
    void resize(int width, int height) override {
        if(width == 0 || height == 0) {
            glfwGetFramebufferSize(getWindow(), &width, &height);
            glfwWaitEvents();
        }

        // NOTE: This is a small hack to mitigate issue that I'm experiencing
        // Basically, when resizing window too much, it causes it to lag (at least on Wayland)
        // By skipping 10 frames between resizing, it helps mitigate the lag of
        // continuous swapchain recreation, and it should still be fine
        // since I check outdated/suboptimal image in loop.
        // This should not be a thing though.
        if(frame % 10 == 0) {
            swapchain->recreate(width, height);
            frame++;
        }
    }

private:
    Device *device;
    Swapchain *swapchain;
    std::unique_ptr<RenderPass> render_pass;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<CommandPool> command_pool;

    std::unique_ptr<Semaphore> image_ready;
    std::unique_ptr<Semaphore> render_finished;
    std::unique_ptr<Fence> in_flight_fence;

    vk::CommandBuffer graphicsCommandBuffer;

private:
    int frame = 0;
};

int main(void) {
    logging::set_environmental_log_level(LOGLEVEL_DEBUG);

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