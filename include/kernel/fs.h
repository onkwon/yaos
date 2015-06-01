#ifndef __FS_H__
#define __FS_H__

#define NR_DATA_BLOCK			10
#define NR_DATA_BLOCK_DIRECT		(NR_DATA_BLOCK - 3)
#ifdef CONFIG_PAGING
#define RFS_BLOCK_SIZE			PAGE_SIZE
#else
#define RFS_BLOCK_SIZE			64
#endif

#include <types.h>
#include <kernel/lock.h>

struct fop_t {
	//open;
	//close;
	//read;
	//write;
};

typedef struct superblock_t fs_sb_t;
typedef struct inode_t fs_inode_t;

struct iop_t {
	fs_inode_t *(*mknod)(unsigned int mode, struct inode_t *parent);
	int (*mkdir)(char *path);
	int (*create)(char *path, unsigned int mode);
	//rmnod;
	//rmdir;
};

struct superblock_t {
	void *root_inode;
	void *next_inode; /* or number of allocated inodes */
	unsigned int free_inode_count;

	size_t block_size;

	unsigned int count; /* mount count */

	struct fop_t fop;
	struct iop_t iop;

	struct list_t list;

	lock_t lock;
};

struct inode_t {
	unsigned int mode;
	size_t size;
	unsigned int count; /* reference count */

	void *data[NR_DATA_BLOCK];

	struct inode_t *parent;
	struct superblock_t *sb;

	lock_t lock;
} __attribute__((packed));

#define INODE_TYPE_MASK		0xf

enum file_type {
	FT_UNKNOWN	= 0x00,
	FT_FILE		= 0x01,
	FT_DIR		= 0x02,
	FT_DEV		= 0x04,
};

struct dir_t {
	void *inode;
	int type;
	char *name;
} __attribute__((packed));

int sys_create(char *path, unsigned int mode);
int sys_mkdir(char *path);
void fs_init();
int readblk(struct inode_t *inode, unsigned int offset, void *buf, size_t len);
struct inode_t *get_inode(char **path, struct inode_t *parent);

struct dev_t;
int sys_mknod(char *name, unsigned int mode, struct dev_t *dev);

#endif /* __FS_H__ */
