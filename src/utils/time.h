#ifndef __PLAYOS_TIME_H__
#define __PLAYOS_TIME_H__

#include <chrono>
#include "spdlog/spdlog.h"


#define MEASURE_BEGIN(t) auto t = std::chrono::steady_clock::now()
#define MEASURE_END(hit, t) do { \
    auto end = std::chrono::steady_clock::now(); \
    spdlog::debug( hit ": {}", std::chrono::duration_cast<std::chrono::milliseconds>(end - t).count()); \
    t = end; \
} while(true)

uint64_t time_milliseconds();

#endif
