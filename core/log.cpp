#include <fmt/base.h>
#include <log.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

int logging::Logging::LOG_LEVEL = 0;

void logging::set_log_level(int log_level) {
    Logging::LOG_LEVEL = log_level;
}
