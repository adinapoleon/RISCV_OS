#pragma once

#include <stdint.h>

namespace vmm {
    constexpr uint32_t PAGE_SIZE = 4096;
    constexpr uint32_t PTE_COUNT = 1024;
    constexpr uint32_t SATP_MODE_SV32 = 1u << 31;

    using pte_t = uint32_t;

    enum PteFlags : uint32_t {
        PTE_V = 1u << 0,
        PTE_R = 1u << 1,
        PTE_W = 1u << 2,
        PTE_X = 1u << 3,
        PTE_U = 1u << 4,
        PTE_G = 1u << 5,
        PTE_A = 1u << 6,
        PTE_D = 1u << 7,
    };

    struct PageTable {
        pte_t entries[PTE_COUNT];
    };

    static_assert(sizeof(PageTable) == PAGE_SIZE, "Sv32 page tables must be exactly one page");

    bool init();
    uintptr_t root_table_address();
    bool vm_map(uintptr_t va, uintptr_t pa, uint32_t flags);
    bool vm_unmap(uintptr_t va);
    bool translate(uintptr_t va, uintptr_t* pa);

    constexpr uint32_t vpn0(uintptr_t va) {
        return (va >> 12) & 0x3FF;
    }

    constexpr uint32_t vpn1(uintptr_t va) {
        return (va >> 22) & 0x3FF;
    }

    constexpr uint32_t page_offset(uintptr_t va) {
        return va & 0xFFF;
    }

    constexpr pte_t make_pte(uintptr_t pa, uint32_t flags) {
        return ((pa >> 12) << 10) | flags;
    }
}
