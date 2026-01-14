#include "constants.h"
#include "utils.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

LoggerInitializer::LoggerInitializer() {
    auto logger = spdlog::stdout_color_mt("console");
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%H:%M:%S.%e] [tid %t] [%^%l%$] %v");
}
