#ifndef __FS_H__
#define __FS_H__

#define NR_DATA_BLOCK		15
#ifdef CONFIG_PAGING
#define RFS_BLOCK_SIZE		PAGE_SIZE
#else
#define RFS_BLOCK_SIZE		64
#endif

#include <types.h>
#include <lock.h>

struct fop_t {
	//open;
	//close;
	//read;
	//write;
};

typedef struct superblock_t fs_sb_t;
typedef struct inode_t fs_inode_t;

struct iop_t {
	fs_inode_t *(*mknod)(
		fs_sb_t *fs, unsigned int mode, fs_inode_t *parent);
	//rmnod;
	//mkdir;
	//rmdir;
};

struct superblock_t {
	void *root_inode;
	void *next_inode; /* or number of allocated inodes */
	unsigned int free_inode_count;

	unsigned int block_size;

	unsigned int count; /* mount count */

	struct fop_t fop;
	struct iop_t iop;

	struct list_t list;

	spinlock_t lock;
};

struct inode_t {
	unsigned int mode;
	unsigned int size;

	unsigned int count; /* reference count */

	void *data[NR_DATA_BLOCK];

	struct inode_t *parent;
	struct superblock_t *sb;

	struct semaphore lock;
} __attribute__((packed));

enum file_type {
	FT_UNKNOWN,
	FT_FILE,
	FT_DIR,
	FT_CHRDEV,
	FT_BLKDEV,
};

struct dir_t {
	void *inode;
	unsigned int rec_len;
	int type;

	unsigned int name_len;
	char *name;
} __attribute__((packed));

#endif /* __FS_H__ */
