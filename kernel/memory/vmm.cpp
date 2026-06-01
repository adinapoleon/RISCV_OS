#include "memory/vmm.h"
#include "memory/pmm.h"
#include <stddef.h>

extern "C" char _ram_start;
extern "C" char _ram_end;

namespace vmm {
    constexpr uintptr_t UART_BASE = 0x10000000;
    constexpr uintptr_t UART_SIZE = PAGE_SIZE;
    constexpr uintptr_t CLINT_BASE = 0x02000000;
    constexpr uintptr_t CLINT_SIZE = 0x10000;

    static PageTable* kernel_root_table = nullptr;

    static bool is_page_aligned(uintptr_t address) {
        return (address & (PAGE_SIZE - 1)) == 0;
    }

    static bool is_valid_pte(pte_t pte) {
        return (pte & PTE_V) != 0;
    }

    static bool is_leaf_pte(pte_t pte) {
        return (pte & (PTE_R | PTE_W | PTE_X)) != 0;
    }

    static uintptr_t pte_address(pte_t pte) {
        return static_cast<uintptr_t>(pte >> 10) << 12;
    }

    static bool valid_leaf_flags(uint32_t flags) {
        bool has_leaf_permission = (flags & (PTE_R | PTE_W | PTE_X)) != 0;
        bool has_write_without_read = (flags & PTE_W) != 0 && (flags & PTE_R) == 0;
        return has_leaf_permission && !has_write_without_read;
    }

    static uint32_t normalized_leaf_flags(uint32_t flags) {
        uint32_t normalized = flags | PTE_V | PTE_A;
        if ((flags & PTE_W) != 0) {
            normalized |= PTE_D;
        }
        return normalized;
    }

    static void flush_tlb() {
        asm volatile("sfence.vma" ::: "memory");
    }

    static void write_satp(uint32_t value) {
        asm volatile("csrw satp, %0" : : "r"(value) : "memory");
    }

    static uint32_t read_satp() {
        uint32_t value = 0;
        asm volatile("csrr %0, satp" : "=r"(value));
        return value;
    }

    static PageTable* table_from_pte(pte_t pte) {
        return reinterpret_cast<PageTable*>(pte_address(pte));
    }

    static bool map_identity_range(uintptr_t start, uintptr_t end, uint32_t flags) {
        for (uintptr_t addr = start; addr < end; addr += PAGE_SIZE) {
            if (!vm_map(addr, addr, flags)) {
                return false;
            }
        }

        return true;
    }

    bool init() {
        kernel_root_table = static_cast<PageTable*>(pmm::alloc_zeroed_page());
        if (kernel_root_table == nullptr) {
            return false;
        }

        if (!map_identity_range(
            reinterpret_cast<uintptr_t>(&_ram_start),
            reinterpret_cast<uintptr_t>(&_ram_end),
            PTE_R | PTE_W | PTE_X | PTE_G
        )) {
            return false;
        }

        return map_identity_range(
            UART_BASE,
            UART_BASE + UART_SIZE,
            PTE_R | PTE_W | PTE_G
        ) && map_identity_range(
            CLINT_BASE,
            CLINT_BASE + CLINT_SIZE,
            PTE_R | PTE_W | PTE_G
        );
    }

    uintptr_t root_table_address() {
        return reinterpret_cast<uintptr_t>(kernel_root_table);
    }

    bool vm_map(uintptr_t va, uintptr_t pa, uint32_t flags) {
        if (kernel_root_table == nullptr
            || !is_page_aligned(va)
            || !is_page_aligned(pa)
            || !valid_leaf_flags(flags)) {
            return false;
        }

        pte_t* root_pte = &kernel_root_table->entries[vpn1(va)];
        PageTable* leaf_table = nullptr;

        if (!is_valid_pte(*root_pte)) {
            leaf_table = static_cast<PageTable*>(pmm::alloc_zeroed_page());
            if (leaf_table == nullptr) {
                return false;
            }
            *root_pte = make_pte(reinterpret_cast<uintptr_t>(leaf_table), PTE_V);
        } else {
            if (is_leaf_pte(*root_pte)) {
                return false;
            }
            leaf_table = table_from_pte(*root_pte);
        }

        pte_t* leaf_pte = &leaf_table->entries[vpn0(va)];
        if (is_valid_pte(*leaf_pte)) {
            return false;
        }

        *leaf_pte = make_pte(pa, normalized_leaf_flags(flags));
        flush_tlb();
        return true;
    }

    bool vm_unmap(uintptr_t va) {
        if (kernel_root_table == nullptr || !is_page_aligned(va)) {
            return false;
        }

        pte_t root_pte = kernel_root_table->entries[vpn1(va)];
        if (!is_valid_pte(root_pte) || is_leaf_pte(root_pte)) {
            return false;
        }

        PageTable* leaf_table = table_from_pte(root_pte);
        pte_t* leaf_pte = &leaf_table->entries[vpn0(va)];
        if (!is_valid_pte(*leaf_pte)) {
            return false;
        }

        *leaf_pte = 0;
        flush_tlb();
        return true;
    }

    bool translate(uintptr_t va, uintptr_t* pa) {
        if (kernel_root_table == nullptr || pa == nullptr) {
            return false;
        }

        pte_t root_pte = kernel_root_table->entries[vpn1(va)];
        if (!is_valid_pte(root_pte)) {
            return false;
        }

        if (is_leaf_pte(root_pte)) {
            *pa = (pte_address(root_pte) & 0xFFC00000u) | (va & 0x003FFFFFu);
            return true;
        }

        PageTable* leaf_table = table_from_pte(root_pte);
        pte_t leaf_pte = leaf_table->entries[vpn0(va)];
        if (!is_valid_pte(leaf_pte) || !is_leaf_pte(leaf_pte)) {
            return false;
        }

        *pa = pte_address(leaf_pte) | page_offset(va);
        return true;
    }

    void enable_paging() {
        if (kernel_root_table == nullptr) {
            return;
        }

        uint32_t root_ppn = root_table_address() >> 12;
        write_satp(SATP_MODE_SV32 | root_ppn);
        flush_tlb();
    }

    bool paging_enabled() {
        return (satp_value() & SATP_MODE_SV32) != 0;
    }

    uint32_t satp_value() {
        return read_satp();
    }
}
