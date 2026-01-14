#pragma once
#include "spdlog/spdlog.h"

struct LoggerInitializer {
    LoggerInitializer();
};

inline LoggerInitializer loggerInit;

#define LOG spdlog::get("console")
