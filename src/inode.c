#include "../include/inode.h"
#include "../include/block.h"
#include "../include/free.h"
#include "../include/pack.h"
#include "../include/common.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

// Read an inode from disk into the provided struct inode
void read_inode(struct inode *in, int inode_num) {
    unsigned char block[BLOCK_SIZE];
    int block_num = FIRST_INODE_BLOCK + (inode_num / INODES_PER_BLOCK);
    int inode_index = inode_num % INODES_PER_BLOCK;

    if (bread(block_num, block) == NULL) {
        perror("Error reading block");
        return;
    }

    unsigned char *addr = block + (inode_index * INODE_SIZE);
    in->size = read_u32(addr);
    in->owner_id = read_u16(addr + 4);
    in->permissions = read_u8(addr + 6);
    in->flags = read_u8(addr + 7);
    in->link_count = read_u8(addr + 8);
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        in->block_ptr[i] = read_u16(addr + 9 + (i * 2));
    }
    in->ref_count = 0; // in-core only
    in->inode_num = inode_num; // in-core only
}

// Write an inode from the provided struct inode to disk
void write_inode(struct inode *in) {
    unsigned char block[BLOCK_SIZE];
    int block_num = FIRST_INODE_BLOCK + (in->inode_num / INODES_PER_BLOCK);
    int inode_index = in->inode_num % INODES_PER_BLOCK;

    if (bread(block_num, block) == NULL) {
        perror("Error reading block");
        return;
    }

    unsigned char *addr = block + (inode_index * INODE_SIZE);
    write_u32(addr, in->size);
    write_u16(addr + 4, in->owner_id);
    write_u8(addr + 6, in->permissions);
    write_u8(addr + 7, in->flags);
    write_u8(addr + 8, in->link_count);
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        write_u16(addr + 9 + (i * 2), in->block_ptr[i]);
    }

    bwrite(block_num, block);
}

// Get a pointer to an in-core inode for the given inode number
struct inode *iget(int inode_num) {
    struct inode *in = incore_find(inode_num);
    if (in) {
        in->ref_count++;
        return in;
    }

    in = incore_find_free();
    if (!in) {
        return NULL;
    }

    read_inode(in, inode_num);
    in->ref_count = 1;

    return in;
}

// Decrement the reference count on the inode and write to disk if it falls to 0
void iput(struct inode *in) {
    if (in->ref_count == 0) {
        return;
    }

    in->ref_count--;

    if (in->ref_count == 0) {
        write_inode(in);
    }
}

// Allocate a new inode and return a pointer to its in-core version
struct inode *ialloc(void) {
    unsigned char block[BLOCK_SIZE];

    if (bread(0, block) == NULL) {
        perror("Error reading block");
        return NULL;
    }

    int inode_num = find_free(block);
    if (inode_num == -1) {
        printf("No free inodes\n");
        return NULL;
    }

    struct inode *in = iget(inode_num);
    if (!in) {
        return NULL;
    }

    set_free(block, inode_num, 1);
    bwrite(0, block);

    // Initialize the inode
    in->size = 0;
    in->owner_id = 0;
    in->permissions = 0;
    in->flags = 0;
    in->link_count = 0;
    memset(in->block_ptr, 0, sizeof(in->block_ptr));
    in->ref_count = 1;
    in->inode_num = inode_num;

    write_inode(in);

    return in;
}

// Functions for finding and managing in-core inodes
struct inode *incore_find_free(void) {
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if (incore[i].ref_count == 0) {
            return &incore[i];
        }
    }
    return NULL;
}

struct inode *incore_find(unsigned int inode_num) {
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if (incore[i].inode_num == inode_num && incore[i].ref_count > 0) {
            return &incore[i];
        }
    }
    return NULL;
}

void incore_free_all(void) {
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        incore[i].ref_count = 0;
    }
}

// Function to expose the incore array for testing purposes
struct inode* get_incore_array(void) {
    return incore;
}
