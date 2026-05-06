#include "kernel/vmm.h"
#include "kernel/pmm.h"
#include <stddef.h>

namespace vmm {
    static PageTable* kernel_root_table = nullptr;

    bool init() {
        kernel_root_table = static_cast<PageTable*>(pmm::alloc_zeroed_page());
        return kernel_root_table != nullptr;
    }

    uintptr_t root_table_address() {
        return reinterpret_cast<uintptr_t>(kernel_root_table);
    }
}
