#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/dir.h"
#include "../include/block.h"
#include "../include/pack.h"
#include "../include/common.h"

struct directory *directory_open(int inode_num) {
    struct inode *inode = iget(inode_num);
    if (inode == NULL) {
        return NULL;
    }

    struct directory *dir = malloc(sizeof(struct directory));
    if (dir == NULL) {
        iput(inode);
        return NULL;
    }

    dir->inode = inode;
    dir->offset = 0;

    return dir;
}

int directory_get(struct directory *dir, struct directory_entry *ent) {
    if (dir->offset >= dir->inode->size) {
        return -1;  // End of directory
    }

    unsigned int block_index = dir->offset / BLOCK_SIZE;
    unsigned int offset_in_block = dir->offset % BLOCK_SIZE;

    if (block_index >= INODE_PTR_COUNT) {
        return -1;  // Block index out of range
    }

    int data_block_num = dir->inode->block_ptr[block_index];
    if (data_block_num < 0) {
        return -1;  // No data block allocated
    }

    unsigned char block[BLOCK_SIZE];
    if (bread(data_block_num, block) == NULL) {
        return -1;  // Failed to read block
    }

    unsigned char *entry_ptr = block + offset_in_block;

    ent->inode_num = read_u16(entry_ptr);
    strcpy(ent->name, (char *)(entry_ptr + 2));


    dir->offset += DIRECTORY_ENTRY_SIZE;

    return 0;  // Success
}

void directory_close(struct directory *dir) {
    iput(dir->inode);
    free(dir);
}

void ls(void) {
    struct directory *dir;
    struct directory_entry ent;

    dir = directory_open(0);
    if (dir == NULL) {
        fprintf(stderr, "Failed to open directory.\n");
        return;
    }

    while (directory_get(dir, &ent) != -1) {
        printf("%d %s\n", ent.inode_num, ent.name);
    }

    directory_close(dir);
}
