#define _XOPEN_SOURCE 700

#include "mlpt.h"

#include <assert.h>
#include <math.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

// Page table entry size and bits
#define PTE_SIZE 8
#define PTE_BITS 3

// Number of bits used per table level
#define BIT_PER_TABLE_LEVEL (POBITS - PTE_BITS)

// Page table base register - stores the root of the page table
size_t ptbr = 0;

// Check if a page table entry is valid (bit 0 is set)
int is_valid(size_t a) {
    return a & 1;
}

// Create a bit mask with n_bits consecutive 1s
size_t construct_mask(size_t n_bits) {
    size_t mask = 1;
    for (size_t i = 0; i < n_bits - 1; i++) {
        mask = (mask << 1) + (size_t)1;
    }
    return mask;
}

// Extract the page offset from a virtual address
size_t extract_offset(size_t a) {
    return a & construct_mask(POBITS);
}

// Extract the VPN (Virtual Page Number) part for a specific level
size_t extract_vpn_part(size_t va, size_t level) {
    size_t shift_amount = POBITS + ((LEVELS - 1 - level) * BIT_PER_TABLE_LEVEL);
    return (va >> shift_amount) & construct_mask(BIT_PER_TABLE_LEVEL);
}

// Allocate aligned memory for a page table or page
size_t allocate_memory() {
    void* ptr;

    // Allocate memory aligned to page boundary
    int result = posix_memalign(&ptr, pow(2, POBITS), pow(2, POBITS));

    if (result != 0) {
        fprintf(stderr, "posix_memalign failed: %d\n", result);
        return 0;
    }

    // Initialize allocated memory to zero
    memset(ptr, 0, pow(2, POBITS));

    return (size_t)ptr;
}

// Translate virtual address to physical address using page table
size_t translate(size_t va) {
    // Return error if page table base register is not set
    if (!ptbr) {
        return ~(size_t)0;
    }

    // Extract page offset from virtual address
    size_t offset = extract_offset(va);

    // Start from root of page table
    size_t* cur = (size_t*)ptbr;

    // Walk through each level of the page table
    for (size_t level = 0; level < LEVELS; level++) {
        size_t vpn = extract_vpn_part(va, level);
        size_t data = cur[vpn];
        if (is_valid(data)) {
            // Move to next level page table
            cur = (size_t*)(data & (~(size_t)1));
        } else {
            // Page fault - entry not valid
            return ~(size_t)0;
        }
    }

    // Return physical address with offset
    return (size_t)cur | offset;
}

// Allocate a page for the given virtual address
int allocate_page(size_t va) {
    // Only allocate pages (offset must be 0)
    if (extract_offset(va) != 0) {
        return -1;
    }

    // Initialize page table base register if not set
    if (!ptbr) {
        ptbr = allocate_memory();
    }

    size_t* cur = (size_t*)ptbr;

    int is_allocated = 0;

    // Walk through page table levels, allocating as needed
    for (size_t level = 0; level < LEVELS; level++) {
        size_t vpn = extract_vpn_part(va, level);

        size_t data = cur[vpn];

        if (is_valid(data)) {
            // Entry exists, move to next level
            cur = (size_t*)(data & (~(size_t)1));
        } else {
            // Entry doesn't exist, allocate new page table
            is_allocated = 1;
            size_t new_addr = allocate_memory();
            cur[vpn] = new_addr | 1;  // Set valid bit
            cur = (size_t*)new_addr;
        }
    }

    return is_allocated;
}

// Deallocate a page for the given virtual address
int deallocate_page(size_t va) {
    // Only deallocate pages (offset must be 0)
    if (extract_offset(va) != 0) {
        return -1;
    }

    // Return 0 if page table base register is not set
    if (!ptbr) {
        return 0;
    }

    size_t* cur = (size_t*)ptbr;

    // Walk through page table levels to find the target page
    for (size_t level = 0; level < LEVELS; level++) {
        size_t vpn = extract_vpn_part(va, level);
        size_t data = cur[vpn];

        if (is_valid(data)) {
            if (level == LEVELS - 1) {
                // Extract the physical address (remove valid bit)
                void* page_addr = (void*)(data & (~(size_t)1));
                
                // Free the physical page
                free(page_addr);
                
                // Clear the page table entry
                cur[vpn] = 0;
                return 1;
            } else {
                // Move to next level page table
                cur = (size_t*)(data & (~(size_t)1));
            }
        } else {
            // Page is not allocated
            return 0;
        }
    }

    // Should not reach here, but return 0 if page not found
    return 0;
}
