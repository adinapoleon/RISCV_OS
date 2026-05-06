#include "kernel/mem.h"
#include <stdint.h>

namespace mem {
    void zero(void* ptr, size_t size) {
        uint8_t* bytes = static_cast<uint8_t*>(ptr);
        for (size_t i = 0; i < size; i++) {
            bytes[i] = 0;
        }
    }
}
