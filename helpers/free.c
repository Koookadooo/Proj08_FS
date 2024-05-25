#include "../include/free.h"
#include <stdio.h>

void set_free(unsigned char *block, int num, int set) {
    int byte_num = num / 8;
    int bit_num = num % 8;
    if (set) {
        block[byte_num] |= (1 << bit_num);
    } else {
        block[byte_num] &= ~(1 << bit_num);
    }
}

int find_free(unsigned char *block) {
    for (int i = 0; i < 4096; i++) {
        for (int j = 0; j < 8; j++) {
            if ((block[i] & (1 << j)) == 0) {
                return i * 8 + j;
            }
        }
    }
    return -1;
}
