#include <kernel/fs.h>
#include <foundation.h>
#include <stdlib.h>

struct superblock_t rootfs;

struct inode_t *rootfs_mknod(struct superblock_t *sb, unsigned int mode,
		struct inode_t *parent)
{
	struct inode_t *new = (struct inode_t *)kmalloc(sizeof(struct inode_t));

	if (!new)
		return NULL;

	new->mode = mode;
	new->size = 0;
	new->count = 0;
	new->parent = parent;
	if (!parent)
		new->parent = new;
	new->sb = sb;

	//semaphore_init();

	int i;
	for (i = 0; i < NR_DATA_BLOCK; i++)
		new->data[i] = NULL;

	sb->next_inode = ((char *)sb->next_inode) + 1;

	return new;
}

/*
unsigned long rootfs_mkdir(struct fs_sb_t *fs)
{
}
*/

void fs_init()
{
	rootfs.next_inode = (void *)0;
	rootfs.free_inode_count = 0;
	rootfs.block_size = RFS_BLOCK_SIZE;
	rootfs.iop.mknod = rootfs_mknod;
	LIST_LINK_INIT(&rootfs.list);
	spinlock_init(rootfs.lock);

	struct inode_t *inode = rootfs.iop.mknod(&rootfs, 0, NULL);
	rootfs.root_inode = (void *)inode;
}
