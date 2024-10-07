#include "window.hpp"

#include <GLFW/glfw3.h>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_funcs.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "log.hpp"
#include "validation.hpp"
#include "vkswapchain.hpp"

Window::Window(std::string title, const std::vector<std::tuple<int, int>> &hints) : title(title)
{
    if(!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW!");

    LOG_DEBUG("Initialized GLFW.");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    for(const auto &[hint, value] : hints) {
        glfwWindowHint(hint, value);
    }

    width = 1280;
    height = 720;

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    if(!window)
        throw std::runtime_error("Failed to create window!");

    LOG_DEBUG("Created window.");

    glfwSetWindowUserPointer(window, this);
    auto resize = [](GLFWwindow *window, int width, int height) {
        static_cast<Window*>(glfwGetWindowUserPointer(window))->resize(width, height);
    };
    glfwSetWindowSizeCallback(window, resize);
    auto mouseWheel = [](GLFWwindow *window, double dx, double dy) {
        static_cast<Window*>(glfwGetWindowUserPointer(window))->mouseScroll(dx, dy);
    };
    glfwSetScrollCallback(window, mouseWheel);
    auto keyboardCall = [](GLFWwindow *window, int key, int scancode, int action, int mod) {
        static_cast<Window*>(glfwGetWindowUserPointer(window))->keyboardCallback(key, action, scancode, mod);
    };
    glfwSetKeyCallback(window, keyboardCall);
}

Window::~Window()
{
    LOG_DEBUG("Stopping GLFW.");

    if(vk_ready) {
        v_instance.destroySurfaceKHR(v_surface, nullptr, v_dispatcher);

        v_instance.destroyDebugUtilsMessengerEXT(v_messenger, nullptr, v_dispatcher);
        LOG_DEBUG("Destroyed Vulkan debug messenger.");
        v_instance.destroy(nullptr, v_dispatcher);
        LOG_DEBUG("Destroyed Vulkan instance.");
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::initVulkan(std::vector<const char*> requestedExtensions, bool portability) {
    auto appinfo = vk::ApplicationInfo()
        .setPApplicationName(title.c_str())
        .setApplicationVersion(vk::makeApiVersion(0, 0, 1, 0))
        .setApiVersion(vk::ApiVersion13)
        .setEngineVersion(vk::makeApiVersion(0, 0, 1, 0))
        .setPEngineName("svk");

    std::vector<const char*> enabledLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    if(Validation::enableValidationLayers) {
        requestedExtensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    if(portability) {
        requestedExtensions.push_back(vk::KHRPortabilityEnumerationExtensionName);
    }

    uint32_t glfwExtensionCount;

    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for(uint32_t i = 0; i < glfwExtensionCount; i++) {
        requestedExtensions.push_back(glfwExtensions[i]);
    }

    auto instanceInfo = vk::InstanceCreateInfo()
        .setPApplicationInfo(&appinfo)
        .setPEnabledExtensionNames(requestedExtensions);
    
    if(Validation::enableValidationLayers) {
        instanceInfo = instanceInfo.setPEnabledLayerNames(enabledLayers);
    } else {
        instanceInfo = instanceInfo.setEnabledLayerCount(0);
    }

    if(portability) {
        instanceInfo.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
    }

    v_dispatcher.init();

    v_instance = vk::createInstance(instanceInfo, nullptr, v_dispatcher);
    v_dispatcher.init(v_instance);

    LOG_DEBUG("Initialized Vulkan instance.");

    if(Validation::enableValidationLayers) {
        auto messengerInfo = vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
            ).setMessageType(
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
            ).setPfnUserCallback(logging::debugCallback);
        
        v_messenger = v_instance.createDebugUtilsMessengerEXT(messengerInfo, nullptr, v_dispatcher);
        LOG_DEBUG("Initialized Vulkan debug messenger.");
    }

    if(glfwCreateWindowSurface(
        static_cast<VkInstance>(v_instance),
        window,
        nullptr,
        reinterpret_cast<VkSurfaceKHR*>(&v_surface)
    ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

    LOG_DEBUG("Created VkSurfaceKHR.");

    vk_ready = true;
}

std::unique_ptr<Device> Window::requestDevice(
    const vk::PhysicalDeviceFeatures &requestedFeatures,
    const std::vector<const char*> &requestedExtensions
) {
    return std::make_unique<Device>(
        v_instance,
        v_surface,
        vk::PhysicalDeviceFeatures(),
        requestedExtensions,
        v_dispatcher
    );
}

std::unique_ptr<Swapchain> Window::requestSwapchain(
    Device &device,
    PreferredSwapchainSettings preferredSettings
) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    return std::make_unique<Swapchain>(device, width, height, v_surface, preferredSettings, v_dispatcher);
}
