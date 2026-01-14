#pragma once
#include "spdlog/spdlog.h"

struct LoggerInitializer {
    LoggerInitializer();
};

inline LoggerInitializer loggerInit;

#define _LOG spdlog::get("console")
