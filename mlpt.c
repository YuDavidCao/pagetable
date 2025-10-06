#include "mlpt.h"
#include "config.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define PTE_SIZE 8
#define PTE_BITS 3

int main() {
    size_t bits_needed_each_level = pow(2, POBITS - PTE_BITS) / PTE_SIZE;
    printf("Bits needed for each level: %zu\n", bits_needed_each_level);
    return 0;
}