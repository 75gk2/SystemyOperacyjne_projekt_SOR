#pragma once
#include "spdlog/spdlog.h"

namespace logging {
    void init();
}

#define _LOG spdlog::get("console")
