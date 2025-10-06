#define _XOPEN_SOURCE 700

#include "mlpt.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

#define PTE_SIZE 8
#define PTE_BITS 3

#define BIT_PER_TABLE (POBITS - PTE_BITS)

size_t ptbr = 0;
size_t page_size = 0;

int is_valid(size_t a) {
    return a & 1;
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
}

int main() {
    page_size = pow(2, POBITS);
    printf("Bits needed for each level: %zu\n", page_size);
    return 0;
}