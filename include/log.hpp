#pragma once

#include <fmt/format.h>
#include <vulkan/vulkan.hpp>

#define LOGLEVEL_DEBUG 4
#define LOGLEVEL_INFO 3
#define LOGLEVEL_WARN 2
#define LOGLEVEL_ERROR 1
#define LOGLEVEL_NONE 0

#define LOG_DEBUG(...) if(logging::Logging::LOG_LEVEL >= LOGLEVEL_DEBUG) { fmt::println(">\tDEBUG:\t{}", fmt::format(__VA_ARGS__)); }
#define LOG_INFO(...) if(logging::Logging::LOG_LEVEL >= LOGLEVEL_INFO) { fmt::println(">>\tINFO:\t{}", fmt::format(__VA_ARGS__)); }
#define LOG_WARN(...) if(logging::Logging::LOG_LEVEL >= LOGLEVEL_WARN) { fmt::println(">>>\tWARN:\t{}", fmt::format(__VA_ARGS__)); }
#define LOG_ERROR(...) if(logging::Logging::LOG_LEVEL >= LOGLEVEL_ERROR) { fmt::println(">>>>\tERROR:\t{}", fmt::format(__VA_ARGS__)); }

namespace logging {
    // TODO: Implement instance-based logging sometime in the future
    class Logging {
    public:
        static int LOG_LEVEL;
    };

    void set_log_level(int log_level);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {
        if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            LOG_DEBUG("Validation layer: {}", pCallbackData->pMessage);
        } else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            LOG_INFO("Validation layer: {}", pCallbackData->pMessage);
        } else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            LOG_WARN("Validation layer: {}", pCallbackData->pMessage);
        } else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            LOG_ERROR("Validation layer: {}", pCallbackData->pMessage);
        }

        return vk::False;
    }
}
