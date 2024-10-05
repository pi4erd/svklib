#include "window.hpp"
#include "log.hpp"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_structs.hpp>

class App : public Window {
public:
    App() : Window("Window Example", {}) {
        Validation::enableValidationLayers = true;
        
        initVulkan({});
        device = requestDevice(vk::PhysicalDeviceFeatures());
        LOG_INFO("Chosen physical device {}", device->v_physical_device.getProperties(v_dispatcher).deviceName.data());
    }

    void loop(double delta) {
        
    }

private:
    std::unique_ptr<Device> device;
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