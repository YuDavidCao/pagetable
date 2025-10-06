#define _XOPEN_SOURCE 700

#include "mlpt.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#define PTE_SIZE 8
#define PTE_BITS 3

#define BIT_PER_TABLE_LEVEL (POBITS - PTE_BITS)

size_t ptbr = 0;
size_t page_size = 0;

int is_valid(size_t a) {
    return a & 1;
}

size_t construct_mask(size_t n_bits) {
    size_t mask = 1;
    for (size_t i = 0; i < n_bits - 1; i++) {
        mask = (mask << 1) + (size_t)1;
    }
    return mask;
}

size_t extract_offset(size_t a) {
    return a & construct_mask(POBITS);
}

size_t extract_vpn_part(size_t va, size_t level) {
    size_t shift_amount = POBITS + ((LEVELS - 1 - level) * BIT_PER_TABLE_LEVEL);
    return (va >> shift_amount) & construct_mask(BIT_PER_TABLE_LEVEL);
}

size_t allocate_memory() {
    void* ptr;

    int result = posix_memalign(&ptr, page_size, page_size);

    if (result != 0) {
        fprintf(stderr, "posix_memalign failed: %d\n", result);
        return 0;
    }

    memset(ptr, 0, page_size);

    return (size_t)ptr;
}

size_t translate(size_t va) {
    if (!ptbr) {
        return ~(size_t)0;
    }
    
    size_t offset = extract_offset(va);
    size_t* cur = (size_t*)ptbr;

    for (size_t i = 0; i < LEVELS; i++) {
        size_t vpn = extract_vpn_part(va, i);
        size_t data = cur[vpn];
        if (is_valid(data)) {
            cur = (size_t*)(data & (~(size_t)1));
        } else {
            return ~(size_t)0;
        }
    }

    return (size_t)(cur + offset);
}

int allocate_page(size_t va) {
    if (extract_offset(va) != 0) {
        return -1;
    }

    if (!ptbr) {
        ptbr = allocate_memory();
    }

    size_t* cur = (size_t*)ptbr;

    int is_allocated = 0;

    for (size_t i = 0; i < LEVELS; i++) {
        size_t vpn = extract_vpn_part(va, i);

        size_t data = cur[vpn];

        if (is_valid(data)) {
            cur = (size_t*)(data & (~(size_t)1));
        } else {
            is_allocated = 1;
            size_t new_addr = allocate_memory();
            cur[vpn] = new_addr | 1;
            cur = (size_t*)new_addr;
        }
    }

    return is_allocated;
}

int main() {
    page_size = pow(2, POBITS);
    printf("Bits needed for each level: %zu\n", page_size);
    return 0;
}