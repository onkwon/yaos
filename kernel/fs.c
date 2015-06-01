#include <kernel/fs.h>
#include <kernel/page.h>
#include <kernel/lock.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>

struct superblock_t rootfs;

static struct inode_t *rootfs_mknod(unsigned int mode, struct inode_t *parent)
{
	struct inode_t *new;

	if ((new = (struct inode_t *)kmalloc(sizeof(struct inode_t))) == NULL)
		return NULL;

	if (parent == NULL) { /* root */
		new->parent = new;
		new->sb = &rootfs;
	} else {
		new->parent = parent;
		new->sb = parent->sb;
		parent->count++;
	}
	new->mode = mode;
	new->size = 0;
	new->count = 0;
	INIT_LOCK(new->lock);

	int i;
	for (i = 0; i < NR_DATA_BLOCK; i++)
		new->data[i] = NULL;

	write_lock(new->sb->lock);
	new->sb->next_inode = ((char *)new->sb->next_inode) + 1;
	write_unlock(new->sb->lock);

	return new;
}

/* alloc new block if null */
#define newblk(blk, size) \
	if (!blk) { \
		blk = (unsigned int)kmalloc(size); \
		if (!blk) return 0; \
		memset((void *)blk, 0, size); \
	}
#define getblk(blk, offset) \
	((unsigned int *)&((unsigned int *)(blk))[offset])

static inline unsigned int *getblk_idx(struct inode_t *inode,
		unsigned int offset)
{
	unsigned int index, nr_entry;
	unsigned int *blk;
	size_t block_size;

	block_size = inode->sb->block_size;
	index = offset / block_size;

	if (index >= NR_DATA_BLOCK_DIRECT) {
		index -= NR_DATA_BLOCK_DIRECT;
		nr_entry = block_size / sizeof(int);

		if (index < nr_entry) { /* single indirect */
			blk = (unsigned int *)
				&inode->data[NR_DATA_BLOCK_DIRECT];

			newblk(*blk, block_size);
			blk = getblk(*blk, index);
		} else if (index < (nr_entry * nr_entry)) { /* doubl indirect */
			blk = (unsigned int *)
				&inode->data[NR_DATA_BLOCK_DIRECT+1];

			newblk(*blk, block_size);
			index -= nr_entry;
			blk = getblk(*blk, index / nr_entry);

			newblk(*blk, block_size);
			blk = getblk(*blk, index % nr_entry);
		} else { /* triple indirect */
			blk   = (unsigned int *)
				&inode->data[NR_DATA_BLOCK_DIRECT+2];

			newblk(*blk, block_size);
			index -= nr_entry * nr_entry;
			blk = getblk(*blk, index / (nr_entry * nr_entry));

			newblk(*blk, block_size);
			blk = getblk(*blk, index %
					(nr_entry * nr_entry) / nr_entry);

			newblk(*blk, block_size);
			blk = getblk(*blk, index %
					(nr_entry * nr_entry) % nr_entry);
		}
	} else {
		blk = (unsigned int *)&inode->data[index];
	}

	return blk;
}

static inline int writeblk(struct inode_t *inode, void *data, size_t len)
{
	char *d, *s = (char *)data;
	unsigned int *blk, offset;
	size_t block_size;

	block_size = inode->sb->block_size;

	for (offset = 0; offset < len; offset++) {
		if ((blk = getblk_idx(inode, inode->size)) == NULL)
			return -ERR_ALLOC;

		if (!*blk) { /* if not allocated yet */
			*blk = (unsigned int)kmalloc(block_size);
			if(!*blk) return -ERR_ALLOC;
		}

		write_lock(inode->lock);
		d = (char *)((unsigned int)*blk + inode->size % block_size);

		*d++ = *s++;
		inode->size++;
		write_unlock(inode->lock);
	}

	return 0;
}

//static inline int
int
readblk(struct inode_t *inode, unsigned int offset, void *buf, size_t len)
{
	unsigned int *blk;
	char *s, *d;
	size_t block_size;

	block_size = inode->sb->block_size;

	for (d = (char *)buf; len && (offset < inode->size); len--) {
		if ((blk = getblk_idx(inode, offset)) == NULL)
			return -ERR_ALLOC;

		read_lock(inode->lock);
		s = (char *)((unsigned int)*blk + offset % block_size);

		*d++ = *s++;
		read_unlock(inode->lock);

		offset++;
	}

	return (int)((unsigned int)d - (unsigned int)buf);
}

struct inode_t *get_inode(char **path, struct inode_t *parent)
{
	struct dir_t dir;
	unsigned int offset;
	char *pwd;

	if (parent == NULL)
		parent = (struct inode_t *)rootfs.root_inode;

	/* skip the first '/', root */
	if (**path == '/')
		(*path)++;

	for (pwd = strtok(*path, "/"); *pwd; pwd = strtok(NULL, "/")) {
		/* if not directory, it must be the last entry of the path.
		 * so the inode requested found. */
		if (!(parent->mode & FT_DIR))
			break;

		for (offset = 0; offset < parent->size;
				offset += sizeof(struct dir_t)) {
			readblk(parent, offset, &dir, sizeof(struct dir_t));

			if (!strcmp(pwd, dir.name))
				break;
		}
		/* such path isn't exist if it reached at the end of dir list */
		if (offset >= parent->size)
			break;

		/* or move on the next path */
		parent = (struct inode_t *)dir.inode;
	}

	/* remove '/' at the end of path if exist */
	if (pwd[strlen(pwd)-1] == '/')
		pwd[strlen(pwd)-1] = '\0';

	/* if path is null after all, the inode requested through path is found.
	 * Or remained path indicates broken, no such directory. return the last
	 * one found. */
	*path = pwd;
	return parent;
}

int sys_create(char *path, unsigned int mode)
{
	struct inode_t *curr, *new;
	struct dir_t dir;
	char *path_copy, *str, *name;
	int err = 0;

	if ((path_copy = (char *)kmalloc(strlen(path)+1)) == NULL)
		return -ERR_ALLOC;

	strncpy(path_copy, path, strlen(path)+1);
	str = path_copy;

	/* if absolute path */
	curr = get_inode(&str, NULL);
	/* or relative path */

	if (!str) { /* already exist */
		err = -ERR_EXIST;
		goto out;
	}

	/* not exsiting parent dir(s) or not a dir */
	if (toknum(str, "/") || !(curr->mode & FT_DIR)) {
		err = -ERR_MKDIR;
		goto out;
	}

	if (!(new = curr->sb->iop.mknod(mode, curr)) ||
			!(name = (char *)kmalloc(strlen(str)+1))) {
		err = -ERR_ALLOC;
		goto out;
	}

	strncpy(name, str, strlen(str)+1);

	dir.inode = new;
	dir.type  = mode & INODE_TYPE_MASK;
	dir.name  = name;

	writeblk(curr, &dir, sizeof(struct dir_t));

out:
	kfree(path_copy);
	return err;
}

int sys_mkdir(char *path)
{
	return sys_create(path, FT_DIR);
}

#include <kernel/init.h>

void __init fs_init()
{
	rootfs.next_inode = (void *)0;
	rootfs.free_inode_count = 0;
	rootfs.block_size = RFS_BLOCK_SIZE;
	rootfs.iop.mknod = rootfs_mknod;
	rootfs.iop.mkdir = sys_mkdir;
	rootfs.iop.create = sys_create;
	list_link_init(&rootfs.list);
	INIT_LOCK(rootfs.lock);

	struct inode_t *inode = rootfs.iop.mknod(FT_DIR, NULL);
	rootfs.root_inode = (void *)inode;

	sys_mkdir("/dev");
}

#include <kernel/module.h>

#define DEVDIR		"/dev/"
#define SUFFIX_MAXLEN	10

int sys_mknod(char *name, unsigned int mode, struct dev_t *dev)
{
	char *buf, *suffix, str[SUFFIX_MAXLEN] = { 0, };
	unsigned int id = 0;
	int err;

	if (dev)
		id = dev->id;

	suffix = itoa(MINOR(id), str, 10, SUFFIX_MAXLEN);

	if ((buf = (char *)kmalloc(strlen(name) + strlen(suffix) +
					strlen(DEVDIR) + 1)) == NULL)
		return -ERR_ALLOC;

	sprintf(buf, DEVDIR "%s%s", name, suffix);
	err = sys_create(buf, mode);

	if (err)
		return err;

	struct inode_t *inode;
	char *t = buf;

	inode = get_inode(&t, NULL);

	if (t) {
		//delete(path);
		return -ERR_PATH;
	}

	inode->data[0] = (void *)id;

	kfree(buf);
	return 0;
}
