#include "../include/block.h"
#include "../include/image.h"
#include "../include/free.h"
#include "../include/common.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


unsigned char *bread(int block_num, unsigned char *block) {
    // Seek to the block number and read the block
    if (lseek(image_fd, block_num * BLOCK_SIZE, SEEK_SET) == -1) {
        perror("Error seeking to block");
        return NULL;
    }
    if (read(image_fd, block, BLOCK_SIZE) != BLOCK_SIZE) {
        perror("Error reading block");
        return NULL;
    }
    return block;
}

void bwrite(int block_num, unsigned char *block) {
    // Seek to the block number and write the block
    if (lseek(image_fd, block_num * BLOCK_SIZE, SEEK_SET) == -1) {
        perror("Error seeking to block");
        return;
    }
    if (write(image_fd, block, BLOCK_SIZE) != BLOCK_SIZE) {
        perror("Error writing to block");
        return;
    }
}

int alloc(void) {
    unsigned char block[4096];
    if (bread(1, block) == NULL) {
        return -1;
    }
    int block_num = find_free(block);
    if (block_num == -1) {
        return -1;
    }
    set_free(block, block_num, 1); 
    bwrite(1, block);

    return block_num;
}
