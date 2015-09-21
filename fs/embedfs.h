#ifndef __EMBEDFS_H__
#define __EMBEDFS_H__

#define MAGIC				0xdeafc0de

#define BLOCK_SIZE			64
#define INODE_TABLE_SIZE(sz)		(100 * (sz)) /* 1% of disk size */
#define NAME_MAX			(256 - 1)

#define SUPERBLOCK			0
#define I_BMAP_BLK			1
#define D_BMAP_BLK			2

#define NR2ADDR(n)			(n * BLOCK_SIZE)

#define set_bitmap(n, arr)		arr[n / 8] |= 1 << (n % 8)
#define reset_bitmap(n, arr)		arr[n / 8] &= ~(1 << (n % 8))

#define get_inode_table(nr_inode, base)	\
	((nr_inode * sizeof(struct embed_inode) / BLOCK_SIZE) + base)
#define get_inode_table_offset(nr_inode) \
	(nr_inode * sizeof(struct embed_inode) % BLOCK_SIZE)

#define FT_ROOT				0xffff

struct embed_superblock {
	unsigned short int block_size;

	unsigned int blocks_count;
	unsigned int inodes_count;
	unsigned int free_inodes_count;
	unsigned int free_blocks_count;

	unsigned int first_block;
	unsigned int inode_table;
	unsigned int data_block;

	unsigned int magic;
} __attribute__((packed));

struct embed_inode {
	unsigned short int addr;
	unsigned short int mode;
	size_t size;
	unsigned int data[NR_DATA_BLOCK];
} __attribute__((packed));

struct embed_dir {
	unsigned short int inode;
	unsigned short int rec_len;
	unsigned char type;
	unsigned char name_len;
	char *name;
} __attribute__((packed));

void embedfs_register();

#endif /* __EMBEDFS_H__ */
