#include <cstdlib>
#include <fmt/base.h>
#include <log.hpp>
#include <stdexcept>
#include <string>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

int logging::Logging::LOG_LEVEL = 0;

void logging::set_log_level(int log_level) noexcept {
    Logging::LOG_LEVEL = log_level;
}

const int logging::get_environmental_log_level() noexcept {
    const char *logLevel = std::getenv("LOG_LEVEL");

    if(logLevel == nullptr) return -1;

    try {
        return std::stoi(logLevel);
    } catch(std::invalid_argument &error) {
        LOG_ERROR("Failed to get environmental log level: {}", error.what());
        return LOGLEVEL_NONE;
    }
}

void logging::set_environmental_log_level(int _default) noexcept {
    const int loglevel = get_environmental_log_level();
    if(loglevel == -1) set_log_level(_default);
    else set_log_level(loglevel);
}
