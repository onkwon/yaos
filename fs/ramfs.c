#include <fs/fs.h>
#include <kernel/page.h>
#include <kernel/device.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <bitops.h>
#include "ramfs.h"

#ifdef CONFIG_PAGING
#define RAMFS_BLOCKSIZE			PAGESIZE
#else
#define RAMFS_BLOCKSIZE			64
#endif

static void read_superblock(struct ramfs_superblock *sb, struct device *dev)
{
	memcpy(sb, (void *)dev->base_addr, sizeof(struct ramfs_superblock));
}

static void write_superblock(struct ramfs_superblock *sb, struct device *dev)
{
	memcpy((void *)dev->base_addr, sb, sizeof(struct ramfs_superblock));
}

#include <lib/firstfit.h>

static void *ramfs_malloc(size_t size, struct device *dev)
{
	struct ramfs_superblock sb;
	unsigned int *head;
	void *addr;

	/* lock superblock */

	read_superblock(&sb, dev);

	head = (unsigned int *)sb.addr_next;
	addr = ff_alloc(&head, size);
	sb.addr_next = (unsigned int)head;

	write_superblock(&sb, dev);

	/* unlock superblock */

	return addr;
}

static void ramfs_free(void *addr, struct device *dev)
{
	struct ramfs_superblock sb;
	unsigned int *head;

	/* lock superblock */

	read_superblock(&sb, dev);

	head = (unsigned int *)sb.addr_next;
	ff_free(&head, addr);
	sb.addr_next = (unsigned int)head;

	write_superblock(&sb, dev);

	/* unlock superblock */
}

static struct ramfs_inode *ramfs_mknod(mode_t mode, struct device *dev)
{
	struct ramfs_inode *new;
	unsigned int i;

	if ((new = ramfs_malloc(sizeof(struct ramfs_inode), dev)) == NULL)
		return NULL;

	new->mode = mode;
	new->size = 0;

	for (i = 0; i < NR_DATA_BLOCK; i++)
		new->data[i] = NULL;

	return new;
}

/* If no valid data block, it tries to allocate new one. Read operation always
 * accesses to already allocated ones. */
static inline unsigned int *take_dblock(struct ramfs_inode *inode,
		size_t pos, struct device *dev)
{
	unsigned int nblock = pos / RAMFS_BLOCKSIZE;

	if (nblock < NR_DATA_BLOCK_DIRECT)
		return (unsigned int *)&inode->data[nblock];

	unsigned int nr_entry, depth, length, i;
	unsigned int *blk;

	nblock -= NR_DATA_BLOCK_DIRECT;
	nr_entry = RAMFS_BLOCKSIZE / WORD_SIZE;
	length = 1;

#define mylog2(n)	(fls(n) - 1)
	for (depth = 0; nblock >> (mylog2(nr_entry) * (depth+1)); depth++) {
		length = 1 << (mylog2(nr_entry) * (depth+1));
		nblock -= length;
	}

	blk = (unsigned int *)&inode->data[depth + NR_DATA_BLOCK_DIRECT];

	for (i = 0; i <= depth; i++) {
		if (!*blk && !(*blk = (unsigned int)
					ramfs_malloc(RAMFS_BLOCKSIZE, dev)))
			return 0;
		memset((void *)*blk, 0, RAMFS_BLOCKSIZE);

#define getblk(blk, offset) \
	((unsigned int *)&((unsigned int *)(blk))[offset])

		blk = getblk(*blk, (nblock / length) % nr_entry);
		nblock %= length;
		length = length >> (fls(nr_entry) - 1);
	}

	return blk;
}

static int write_block(struct ramfs_inode *inode, void *data, size_t len,
		struct device *dev)
{
	char *d, *s = (char *)data;
	unsigned int *blk, offset;
	int ret = 0;

	/* lock inode */

	for (offset = 0; offset < len; offset++) {
		if ((blk = take_dblock(inode, inode->size, dev)) == NULL) {
			ret = -ERR_ALLOC;
			break;
		}

		if (!*blk) { /* if not allocated yet */
			*blk = (unsigned int)ramfs_malloc(RAMFS_BLOCKSIZE, dev);
			if (!*blk) {
				ret = -ERR_ALLOC;
				break;
			}
		}

		d = (char *)((unsigned int)*blk +
				inode->size % RAMFS_BLOCKSIZE);

		*d++ = *s++;
		inode->size++;
	}

	/* unlock inode */

	return ret;
}

static size_t read_block(struct ramfs_inode *inode, unsigned int offset,
		void *buf, size_t len, struct device *dev)
{
	unsigned int *blk;
	char *s, *d;

	/* lock inode */

	for (d = (char *)buf; len && (offset < inode->size); len--) {
		if ((blk = take_dblock(inode, offset, dev)) == NULL)
			break;

		s = (char *)((unsigned int)*blk + offset % RAMFS_BLOCKSIZE);

		*d++ = *s++;

		offset++;
	}

	/* unlock inode */

	return (int)((unsigned int)d - (unsigned int)buf);
}

static size_t tok_strlen(const char *s, const char token)
{
	const char *p;

	for (p = s; *p; p++) {
		if (*p == token) {
			p++;
			break;
		}
	}

	return p - s;
}

static const char *lookup(struct ramfs_inode **inode, const char *pathname,
		struct device *dev)
{
	struct ramfs_inode *curr;
	struct ramfs_dir dir;
	struct ramfs_superblock sb;
	unsigned int offset, len, i;
	const char *pwd;

	/* skip '/'s if exist */
	for (i = 0; pathname[i] == '/'; i++) ;

	read_superblock(&sb, dev);
	curr = (struct ramfs_inode *)sb.root_inode;

	for (pwd = pathname + i; *pwd; pwd = pathname + i) {
		len = tok_strlen(pwd, '/');

		/* if not a directory, it must be at the end of path. */
		if (!(curr->mode & FT_DIR))
			break;

		for (offset = 0; offset < curr->size;
				offset += sizeof(struct ramfs_dir)) {
			read_block(curr, offset, &dir,
					sizeof(struct ramfs_dir), dev);

			if (dir.name && len == strnlen(dir.name,
						FILENAME_MAX) &&
					!strncmp(dir.name, pwd, len))
				break;
		}

		/* no such path exist if it reaches at the end of dir list */
		if (offset >= curr->size)
			break;

		/* or move on */
		curr = (struct ramfs_inode *)dir.inode;

		i += len;
	}

	*inode = curr;

	/* if pwd is null after all, the inode you're looking for is found.
	 * or remained path indicates broken, no such directory. return the
	 * last one found. */

	return pathname + i;
}

static int create_file(const char *pathname, mode_t mode, struct device *dev)
{
	struct ramfs_inode *new, *parent;
	struct ramfs_dir dir;
	char *name;
	size_t len;
	const char *p;

	p = lookup(&parent, pathname, dev);

	if (p == NULL) /* already exist */
		return -ERR_DUP;

	/* not exsiting parent dir(s) or not a dir */
	if (toknum(p, "/") || !(parent->mode & FT_DIR))
		return -ERR_PATH;

	len = strnlen(p, FILENAME_MAX);

	if (!(new = ramfs_mknod(mode, dev)) ||
			!(name = ramfs_malloc(len+1, dev)))
		return -ERR_ALLOC;

	strncpy(name, p, len+1);

	dir.inode = new;
	dir.type  = GET_FILE_TYPE(mode);
	dir.name  = name;

	write_block(parent, &dir, sizeof(struct ramfs_dir), dev);

	return 0;
}

static int ramfs_create(struct inode *inode, const char *pathname, mode_t mode)
{
	return create_file(pathname, mode, inode->sb->dev);
}

static size_t ramfs_read(struct file *file, void *buf, size_t len)
{
	size_t count;

	count = read_block((struct ramfs_inode *)file->inode->addr,
			file->offset, buf, len, file->inode->sb->dev);

	file->offset += count;

	return count;
}

static int ramfs_lookup(struct inode *inode, const char *pathname)
{
	struct ramfs_inode *fs_inode;
	const char *s;

	s = lookup(&fs_inode, pathname, inode->sb->dev);

	if (*s)
		return -ERR_PATH;

	inode->addr = (unsigned int)fs_inode;
	inode->mode = fs_inode->mode;
	inode->size = fs_inode->size;

	if (GET_FILE_TYPE(fs_inode->mode) == FT_DEV)
		inode->dev = (dev_t)fs_inode->data[0];

	return 0;
}

static struct inode_operations iops = {
	.lookup = ramfs_lookup,
	.create = ramfs_create,
};

static struct file_operations fops = {
	.open  = NULL,
	.read  = ramfs_read,
	.write = NULL,
	.close = NULL,
	.seek  = NULL,
	.ioctl = NULL,
};

static void ramfs_read_inode(struct inode *inode)
{
	struct ramfs_inode *ramfs_inode;

	ramfs_inode = (struct ramfs_inode *)inode->addr;

	inode->mode = ramfs_inode->mode;
	inode->size = ramfs_inode->size;
	inode->iop = &iops;
	inode->fop = &fops;
}

static int ramfs_mount(struct device *dev)
{
	return 0;
}

static struct super_operations super_ops = {
	.read_inode = ramfs_read_inode,
	.mount = ramfs_mount,
};

struct device *ramfs_build(size_t size, const char *name)
{
	static unsigned int ramfs_major = 0;
	struct device *dev;
	unsigned int root_inode;

	if (!(dev = mkdev(ramfs_major, 0, NULL, name)))
		return NULL;

	ramfs_major = MAJOR(dev->id);

	if (!(dev->base_addr = (unsigned int)kmalloc(size))) {
		remove_device(dev);
		return NULL;
	}

	dev->block_size = RAMFS_BLOCKSIZE;
	dev->nr_blocks = size / RAMFS_BLOCKSIZE;

	struct ramfs_superblock new;
	new.addr_next =
		ALIGN_WORD(dev->base_addr + sizeof(struct ramfs_superblock));
	new.addr_next = (unsigned int)ff_freelist_init((void *)new.addr_next,
			(void *)(dev->base_addr +
				dev->block_size * dev->nr_blocks - 1));
	write_superblock(&new, dev);

	root_inode = (unsigned int)ramfs_mknod(FT_DIR, dev);
	read_superblock(&new, dev);
	new.root_inode = root_inode;
	write_superblock(&new, dev);

	if (!strcmp(name, "/")) /* if root file system */
		create_file("/dev", FT_DIR, dev);

	return dev;
}

static int read_super(struct superblock *sb, struct device *dev)
{
	struct ramfs_superblock fs_sb;

	read_superblock(&fs_sb, dev);
	sb->root_inode = fs_sb.root_inode;
	sb->op = &super_ops;

	return 0;
}

void ramfs_register()
{
	struct file_system_type *fs;

	fs = kmalloc(sizeof(struct file_system_type));
	fs->read_super = read_super;

	add_file_system(fs, "ramfs");
}

#define SUFFIX_MAXLEN	10

int sys_mknod(const char *name, unsigned int mode, dev_t id)
{
	extern struct device *devfs;

	char *buf, *suffix, str[SUFFIX_MAXLEN] = { 0, };
	unsigned int len;
	int ret;

	if (!name)
		return -ERR_RANGE;

	suffix = itoa(MINOR(id), str, 10, SUFFIX_MAXLEN);
	len = strnlen(name, FILENAME_MAX) +
		strnlen(suffix, SUFFIX_MAXLEN);

	if ((buf = (char *)kmalloc(len + 1)) == NULL)
		return -ERR_ALLOC;

	snprintf(buf, len, "%s%s", name, suffix);
	ret = create_file(buf, mode, devfs);

	if (ret)
		return ret;

	struct ramfs_inode *inode;
	const char *s;

	s = lookup(&inode, buf, devfs);

	if (*s) {
		//delete(buf);
		return -ERR_PATH;
	}

	inode->data[0] = (void *)id;

	kfree(buf);

	return 0;
}
