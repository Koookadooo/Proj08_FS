#include <stdio.h>
#include <string.h>
#include "../include/block.h"
#include "../include/free.h"
#include "../include/image.h"
#include "../include/inode.h"
#include "../include/pack.h"
#include "../include/common.h"

void mkfs(void) {
    // Open the image file with truncation
    char *filename = "fs_image.bin";
    if (image_open(filename, 1) < 0) {
        fprintf(stderr, "Failed to open image file %s\n", filename);
        return;
    }

    // Initialize inode and block maps
    unsigned char empty_map[BLOCK_SIZE] = {0};
    bwrite(INODE_MAP_BLOCK, empty_map);
    bwrite(FREE_MAP_BLOCK, empty_map);

    // Initialize inode blocks
    for (int i = 0; i < NUM_INODE_BLOCKS; i++) {
        bwrite(FIRST_INODE_BLOCK + i, empty_map);
    }

    // Allocate a new inode
    struct inode *new_inode = ialloc();
    if (new_inode == NULL) {
        fprintf(stderr, "Failed to allocate new inode.\n");
        image_close();
        return;
    }
    unsigned int inode_num = new_inode->inode_num;

    // Allocate a new data block
    int block_num = alloc();
    if (block_num < 0) {
        fprintf(stderr, "Failed to allocate new data block.\n");
        image_close();
        return;
    }

    // Initialize the inode
    new_inode->flags = 2;  // directory flag
    new_inode->size = DIRECTORY_ENTRY_SIZE * 2;  // 64 bytes
    new_inode->block_ptr[0] = block_num;
    for (int i = 1; i < INODE_PTR_COUNT; i++) {
        new_inode->block_ptr[i] = 0;
    }

    // Create directory entries in-memory
    unsigned char block[BLOCK_SIZE];
    memset(block, 0, BLOCK_SIZE);  // Initialize the block with zeros

    // Entry for "."
    write_u16(block, inode_num);  // inode number
    strcpy((char *)(block + 2), ".");  // file name

    // Entry for ".."
    write_u16(block + DIRECTORY_ENTRY_SIZE, inode_num);  // inode number
    strcpy((char *)(block + DIRECTORY_ENTRY_SIZE + 2), "..");  // file name

    // Write the directory data block out to disk
    bwrite(block_num, block);

    // Write the new directory inode out to disk and free up the in-core inode
    iput(new_inode);
}

void clfs(void) {
    // Close the image file
    char *filename = "fs_image.bin";
    if (image_close() < 0) {
        fprintf(stderr, "Failed to close image file %s\n", filename);
    }
}