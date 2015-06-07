#ifndef __RAMFS_H__
#define __RAMFS_H__

#ifdef CONFIG_PAGING
#define RAMFS_BLOCKSIZE			PAGE_SIZE
#else
#define RAMFS_BLOCKSIZE			64
#endif

#include <types.h>

struct ramfs_superblock {
	unsigned int root_inode;
	unsigned int addr_next;
};

struct ramfs_inode {
	mode_t mode;
	size_t size;
	void *data[NR_DATA_BLOCK];
} __attribute__((packed));

struct ramfs_dir {
	void *inode;
	unsigned char type;
	char *name;
} __attribute__((packed));

void ramfs_register();
struct device *ramfs_build(size_t size, const char *name);

#endif /* __RAMFS_H__ */
