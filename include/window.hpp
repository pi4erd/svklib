#pragma once

#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <tuple>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

class Window {
public:
    Window(std::string title, const std::vector<std::tuple<int, int>> &hints);
    virtual ~Window();

    void initVulkan(std::vector<const char*> requestedExtensions, bool enableValidationLayers=false);

    bool shouldClose(void) { return glfwWindowShouldClose(window); }
    void close(void) { glfwSetWindowShouldClose(window, true); }
    void pollEvents(void) { glfwPollEvents(); }

    void toggleFullscreen() {
        if(!fullscreen) {
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            int width, height, xpos, ypos;
            glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);

            glfwSetWindowMonitor(window, monitor, xpos, ypos, width, height, GLFW_DONT_CARE);
            fullscreen = true;
        } else {
            glfwSetWindowMonitor(window, nullptr, 0, 0, 1280, 720, GLFW_DONT_CARE);
            fullscreen = false;
        }
    }

    int width, height;

protected: // Virtuals
    virtual void mouseScroll(double dx, double dy) {
        (void)dx, (void)dy;
    }
    virtual void resize(int width, int height) {
        this->width = width;
        this->height = height;
    }
    virtual void keyboardCallback(int key, int action, int scancode, int mod) {
        (void)key;
        (void)action;
        (void)scancode;
        (void)mod;
    }
    virtual void mouseButton(int key, int action, int mod) {
        (void)key;
        (void)action;
        (void)mod;
    }
    GLFWwindow *getWindow(void) { return window; }

public: // vulkan properties
    vk::Instance v_instance;
    vk::DebugUtilsMessengerEXT v_messenger;
    vk::SurfaceKHR v_surface;
    vk::DispatchLoaderDynamic v_dispatcher;

private:
    bool fullscreen = false;

    GLFWwindow *window;
    std::string title;
};
