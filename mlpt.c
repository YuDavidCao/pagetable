#define _XOPEN_SOURCE 700

#include "mlpt.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

void page_table_init() {
    void *ptr;

    int result = posix_memalign(&ptr, page_size, page_size);

    if (result != 0) {
        fprintf(stderr, "posix_memalign failed: %d\n", result);
        return;
    }

    memset(ptr, 0, page_size);

    ptbr = (size_t)ptr;
}

int allocate_page(size_t va) {
    if (!ptbr) {
        page_table_init();
    }

    for (size_t i = 0; i < LEVELS; i++) {
        size_t vpn = extract_vpn_part(va, i);
    }
}

int main() {
    page_size = pow(2, POBITS);
    printf("Bits needed for each level: %zu\n", page_size);
    return 0;
}