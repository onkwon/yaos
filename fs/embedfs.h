#ifndef __EMBEDFS_H__
#define __EMBEDFS_H__

struct embed_superblock {
	unsigned int magic;

	unsigned short int block_size;
	char __pad[2];

	unsigned int nr_blocks;
	unsigned int nr_inodes;
	unsigned int free_inodes_count;
	unsigned int free_blocks_count;

	unsigned int first_block; /* the first block of device, base address */
	unsigned int inode_table; /* the first block of inode table */
	unsigned int data_block; /* the first block of data block */
} __attribute__((packed));

struct embed_inode {
	unsigned short int id;
	unsigned short int mode;
	size_t size;
	size_t hole;
	unsigned int data[NR_DATA_BLOCK];
} __attribute__((packed));

struct embed_dir {
	unsigned short int inode;
	unsigned short int rec_len;
	unsigned char type;
	unsigned char name_len;
	char __pad[2];
	char *name;
} __attribute__((packed));

void embedfs_register();

#endif /* __EMBEDFS_H__ */
