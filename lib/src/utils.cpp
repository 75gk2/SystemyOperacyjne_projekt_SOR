#include "constants.hpp"
#include "utils.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>


#include "spdlog/sinks/syslog_sink.h"

LoggerInitializer::LoggerInitializer() {
    // auto logger = spdlog::syslog_logger_mt("test", "SOR", LOG_PID | LOG_CONS, LOG_USER, true); //SYSLOG SINK - for debugging on native system
    const auto logger = spdlog::stdout_color_mt("console");//std::to_string(getpid()));

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%H:%M:%S.%e] [tid %t] [%^%l%$] %v");
}

