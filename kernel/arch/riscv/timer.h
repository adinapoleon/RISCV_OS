#pragma once

#include <stdint.h>

namespace timer {
    void start(uint32_t interval);
    uint32_t ticks();
}
