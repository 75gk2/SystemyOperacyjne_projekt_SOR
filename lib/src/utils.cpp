#include "constants.hpp"
#include "utils.hpp"
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


#include "spdlog/sinks/syslog_sink.h"

LoggerInitializer::LoggerInitializer() {
    // auto logger = spdlog::syslog_logger_mt("test", "SOR", LOG_PID | LOG_CONS, LOG_USER, true); //SYSLOG SINK - for debugging on native system
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("sor.log", true);
    const auto logger = std::make_shared<spdlog::logger>("console",
                                                         spdlog::sinks_init_list{ consoleSink,fileSink});//consoleSink,

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%H:%M:%S.%e] [tid %t] [%^%l%$] %v");
}