/*
    Example for window with swapchain in Vulkan with svklib
*/

#include "window.hpp"
#include "log.hpp"
#include "vkswapchain.hpp"

#include <memory>
#include <stdexcept>

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

        swapchain = requestSwapchain(*device, PreferredSwapchainSettings {
            .requestedCapabilities = vk::SurfaceCapabilitiesKHR(),
            .preferredFormat = vk::Format::eB8G8R8A8Srgb,
            .preferredPresentMode = vk::PresentModeKHR::eFifo
        });
        LOG_INFO("Swapchain created with extent {}x{}", 
            swapchain->v_swapchain_extent.width, swapchain->v_swapchain_extent.height
        );
    }

    void loop(double delta) {
        
    }

private:
    std::unique_ptr<Device> device;
    std::unique_ptr<Swapchain> swapchain;
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