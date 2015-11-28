#include <fs/fs.h>
#include <kernel/page.h>
#include <kernel/lock.h>
#include <kernel/buffer.h>
#include <kernel/device.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include "embedfs.h"

static size_t read_block(unsigned int nr, void *buf, const struct device *dev)
{
	unsigned int disk_block, fs_block, offset;

	fs_block   = BLOCK_BASE(NR2ADDR(nr), BLOCK_SIZE);
	disk_block = BLOCK_BASE(fs_block, dev->block_size);
	offset = fs_block % dev->block_size;

	unsigned int i;
	char *diskbuf, *s, *d;

	diskbuf = getblk_lock(disk_block, dev);

	s = diskbuf + offset;
	d = buf;

	for (i = 0; (i < BLOCK_SIZE) &&
			((diskbuf+i) < (diskbuf+dev->block_size)); i++)
		*d++ = *s++;

	putblk_unlock(disk_block, dev);

	return i;
}

static size_t write_block(unsigned int nr, void *buf, size_t len,
		const struct device *dev)
{
	unsigned int disk_block, fs_block, offset;

	fs_block   = BLOCK_BASE(NR2ADDR(nr), BLOCK_SIZE);
	disk_block = BLOCK_BASE(fs_block, dev->block_size);
	offset = fs_block % dev->block_size;

	unsigned int i;
	char *diskbuf, *s, *d;

	diskbuf = getblk_lock(disk_block, dev);

	s = buf;
	d = diskbuf + offset;

	for (i = 0; (i < len) && ((diskbuf+i) < (diskbuf+dev->block_size)); i++)
		*d++ = *s++;

	updateblk(disk_block, dev);

	putblk_unlock(disk_block, dev);

	return i;
}

static int read_superblock(struct embed_superblock *sb,
		const struct device *dev)
{
	char *buf;

	if ((buf = kmalloc(BLOCK_SIZE)) == NULL)
		return -ERR_ALLOC;

	read_block(SUPERBLOCK, buf, dev);
	memcpy(sb, buf, sizeof(struct embed_superblock));

	kfree(buf);
	return 0;
}

static void write_superblock(struct embed_superblock *sb,
		const struct device *dev)
{
	write_block(SUPERBLOCK, sb, sizeof(struct embed_superblock), dev);
}

static void read_inode_bitmap(char *bitmap, const struct device *dev)
{
	read_block(I_BMAP_BLK, bitmap, dev);
}

static void write_inode_bitmap(char *bitmap, const struct device *dev)
{
	write_block(I_BMAP_BLK, bitmap, BLOCK_SIZE, dev);
}

static unsigned int alloc_free_inode(const struct device *dev)
{
	struct embed_superblock sb;
	unsigned int i, bit, n, len;
	char *bitmap;

	if ((bitmap = kmalloc(BLOCK_SIZE)) == NULL)
		return 0;

	n = 0; /* to avoid compiler warning */

	/* lock superblock */

	read_superblock(&sb, dev);
	read_inode_bitmap(bitmap, dev);

	len = sb.inodes_count / 8;
	for (i = 0; (i < len) && (bitmap[i] == 0xff); i++) ;
	for (bit = 0; (bit < 8) && (bitmap[i] & (1 << bit)); bit++) ;

	if ((i >= len) && (bit > (sb.inodes_count % 8))) {
		i = len;
		if (!(len = sb.inodes_count % 8))
			goto out;

		for (bit = 0; (bit < len) && (bitmap[i] & (1 << bit)); bit++) ;
		if (bit >= len) goto out;
	}

	n = i * 8 + bit;

	set_bitmap(n, bitmap);
	sb.free_inodes_count--;
	write_superblock(&sb, dev);
	write_inode_bitmap(bitmap, dev);

out:
	/* unlock superblock */

	kfree(bitmap);
	return n;
}

static unsigned int alloc_free_block(const struct device *dev)
{
	struct embed_superblock sb;
	char *bitmap;
	unsigned int size, len, bit, i, j, n;

	if ((bitmap = kmalloc(BLOCK_SIZE)) == NULL)
		return 0;

	/* lock superblock */

	read_superblock(&sb, dev);

	size = sb.blocks_count / 8;
	size += (sb.blocks_count % 8)? 1 : 0;
	len = size / BLOCK_SIZE;
	n = 0;

	for (i = 0; i < len; i++) {
		read_block(D_BMAP_BLK + i, bitmap, dev);
		for (j = 0; (j < BLOCK_SIZE) && (bitmap[j] == 0xff); j++) ;
		for (bit = 0; (bit < 8) && (bitmap[j] & (1 << bit)); bit++) ;
		if ((j < BLOCK_SIZE) && (bit < 8)) {
			n = j * 8 + bit;
			set_bitmap(n, bitmap);
			write_block(D_BMAP_BLK + i, bitmap, BLOCK_SIZE, dev);
			break;
		}
	}

	if (!n && (sb.blocks_count % 8)) {
		read_block(D_BMAP_BLK + len, bitmap, dev);
		size = sb.blocks_count % 8;
		for (bit = 0; (bit < size) && (bitmap[0] & (1 << bit)); bit++) ;
		if (bit < size) {
			n = len * 8 + bit;
			set_bitmap(n, bitmap);
			write_block(D_BMAP_BLK+len, bitmap, BLOCK_SIZE, dev);
		}
	}

	if (n) {
		n += sb.data_block;
		sb.free_blocks_count--;
		write_superblock(&sb, dev);
	}

	/* unlock superblock */

	kfree(bitmap);
	return n;
}

static int update_inode_table(struct embed_inode *inode,
		const struct device *dev)
{
	struct embed_superblock sb;
	char *buf;
	unsigned int nblock, offset;

	if ((buf = kmalloc(BLOCK_SIZE)) == NULL)
		return -ERR_ALLOC;

	read_superblock(&sb, dev);

	/* lock superblock */

	nblock = get_inode_table(inode->addr, sb.inode_table);
	offset = get_inode_table_offset(inode->addr);

	unsigned int diff = sizeof(struct embed_inode);

	if ((offset + sizeof(struct embed_inode)) > BLOCK_SIZE) {
		diff = BLOCK_SIZE - offset;
		read_block(nblock+1, buf, dev);
		memcpy(buf, (char *)inode + diff,
				sizeof(struct embed_inode) - diff);
		write_block(nblock+1, buf, BLOCK_SIZE, dev);
	}

	read_block(nblock, buf, dev);
	memcpy(buf+offset, inode, diff);
	write_block(nblock, buf, BLOCK_SIZE, dev);

	/* unlock superblock */

	kfree(buf);
	return 0;
}

static inline unsigned int alloc_zeroed_free_block(const struct device *dev)
{
	unsigned int nblock;
	char *buf;

	if ((buf = kmalloc(BLOCK_SIZE)) == NULL)
		return 0;

	if (!(nblock = alloc_free_block(dev)))
		goto out;

	memset(buf, 0, BLOCK_SIZE);
	write_block(nblock, buf, BLOCK_SIZE, dev);
out:
	kfree(buf);
	return nblock;
}

static inline unsigned int check_n_alloc_block(unsigned int nblock,
		unsigned int index, const struct device *dev)
{
	char *buf;

	if (!nblock)
		return 0;

	if ((buf = kmalloc(BLOCK_SIZE)) == NULL)
		return 0;

	read_block(nblock, buf, dev);
	if (!buf[index]) {
		if (!(buf[index] = alloc_zeroed_free_block(dev))) {
			nblock = 0;
			goto out; /* disk full */
		}
		write_block(nblock, buf, BLOCK_SIZE, dev);
	}
	nblock = buf[index];

out:
	kfree(buf);
	return nblock;
}

static inline unsigned int get_data_block(struct embed_inode *inode,
		unsigned int offset, const struct device *dev)
{
	unsigned int nblock, nr_entry, idx1, idx2, idx3;

	idx1 = offset / BLOCK_SIZE;

	if (idx1 >= NR_DATA_BLOCK_DIRECT) {
		idx1 -= NR_DATA_BLOCK_DIRECT;
		nr_entry = BLOCK_SIZE / WORD_SIZE;

		if (idx1 < nr_entry) { /* single indirect */
			if (!(nblock = inode->data[NR_DATA_BLOCK_DIRECT])) {
				nblock = alloc_zeroed_free_block(dev);
				inode->data[NR_DATA_BLOCK_DIRECT] = nblock;
				update_inode_table(inode, dev);
			}

			nblock = check_n_alloc_block(nblock, idx1, dev);
		} else if (idx1 < (nr_entry * nr_entry)) { /* double indirect */
			if (!(nblock = inode->data[NR_DATA_BLOCK_DIRECT+1])) {
				nblock = alloc_zeroed_free_block(dev);
				inode->data[NR_DATA_BLOCK_DIRECT+1] = nblock;
				update_inode_table(inode, dev);
			}

			idx1 -= nr_entry;
			idx2  = idx1 % nr_entry;
			idx1 /= nr_entry;
			nblock = check_n_alloc_block(nblock, idx1, dev);
			nblock = check_n_alloc_block(nblock, idx2, dev);
		} else { /* triple indirect */
			if (!(nblock = inode->data[NR_DATA_BLOCK_DIRECT+2])) {
				nblock = alloc_zeroed_free_block(dev);
				inode->data[NR_DATA_BLOCK_DIRECT+2] = nblock;
				update_inode_table(inode, dev);
			}

			idx1 -= nr_entry * nr_entry;
			idx2 = idx1 % (nr_entry * nr_entry) / nr_entry;
			idx3 = idx1 % (nr_entry * nr_entry) % nr_entry;
			idx1 /= (nr_entry * nr_entry);
			nblock = check_n_alloc_block(nblock, idx1, dev);
			nblock = check_n_alloc_block(nblock, idx2, dev);
			nblock = check_n_alloc_block(nblock, idx3, dev);
		}
	} else {
		if (!(nblock = inode->data[idx1])) {
			nblock = alloc_zeroed_free_block(dev);
			inode->data[idx1] = nblock;
			update_inode_table(inode, dev);
		}
	}

	return nblock;
}

static int write_data_block(struct embed_inode *inode, const void *data,
		size_t len, const struct device *dev)
{
	unsigned int nblk, prev, offset;
	char *buf, *src;

	if ((buf = kmalloc(BLOCK_SIZE)) == NULL)
		return -ERR_ALLOC;

	src = (char *)data;
	offset = 0;
	prev = 0;
	nblk = 0;

	/* lock inode */

	while (offset < len) {
		if (!(nblk = get_data_block(inode, inode->size, dev)))
			break; /* disk full */

		if (nblk != prev) {
			/* write back before reading the next block */
			if (prev)
				write_block(prev, buf, BLOCK_SIZE, dev);

			read_block(nblk, buf, dev);
		}

		buf[inode->size % BLOCK_SIZE] = *src;

		src++;
		inode->size++;
		offset++;

		prev = nblk;
	}

	if (nblk)
		write_block(nblk, buf, BLOCK_SIZE, dev);

	update_inode_table(inode, dev);

	/* unlock inode */

	kfree(buf);

	return offset;
}

static int read_data_block(struct embed_inode *inode, unsigned int offset,
		void *buf, size_t len, const struct device *dev)
{
	unsigned int nblk;
	char *s, *d, *t;

	if ((t = kmalloc(BLOCK_SIZE)) == NULL)
		return -ERR_ALLOC;

	for (d = (char *)buf; len && (offset < inode->size); len--, offset++) {
		if (!(nblk = get_data_block(inode, offset, dev))) {
			kfree(t);
			return -ERR_RANGE;
		}

		read_block(nblk, t, dev);
		s = t + offset % BLOCK_SIZE;
		*d++ = *s++;
	}

	kfree(t);
	return (int)((unsigned int)d - (unsigned int)buf);
}

static unsigned int make_node(mode_t mode, const struct device *dev)
{
	unsigned int inode, i;
	struct embed_inode new;

	if (!(inode = alloc_free_inode(dev))) {
		if (GET_FILE_TYPE(mode) != FT_ROOT)
			return 0;
	}

	new.addr = inode;
	new.mode = mode;
	new.size = 0;
	for (i = 0; i < NR_DATA_BLOCK; i++) new.data[i] = 0;

	update_inode_table(&new, dev);

	return inode;
}

static int read_inode(struct embed_inode *inode, const struct device *dev)
{
	unsigned int nblock, offset;
	char *buf;

	if ((buf = kmalloc(BLOCK_SIZE)) == NULL)
		return -ERR_ALLOC;

	struct embed_superblock sb;
	unsigned int inode_size = sizeof(struct embed_inode);

	if (read_superblock(&sb, dev)) {
		kfree(buf);
		return -ERR_ALLOC;
	}

	nblock = inode->addr * inode_size / BLOCK_SIZE;
	nblock += sb.inode_table;
	offset = inode->addr * inode_size % BLOCK_SIZE;

	unsigned int diff = inode_size;

	if ((offset + inode_size) > BLOCK_SIZE) {
		diff = BLOCK_SIZE - offset;
		read_block(nblock+1, buf, dev);
		memcpy((char *)inode + diff, buf, inode_size - diff);
	}

	read_block(nblock, buf, dev);
	memcpy(inode, buf+offset, diff);

	kfree(buf);
	return 0;
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

static const char *lookup(struct embed_inode *inode, const char *pathname,
		const struct device *dev)
{
	struct embed_dir dir;
	struct embed_inode curr;
	unsigned int offset, len, i, dir_size;
	const char *pwd;
	char *name;

	dir_size = sizeof(struct embed_dir) - sizeof(char *);

	/* skip '/'s if exist */
	for (i = 0; pathname[i] == '/'; i++) ;

	curr.addr = 0; /* start searching from root */
	if (read_inode(&curr, dev)) {
		debug(MSG_DEBUG, "wrong inode");
		return (char *)-ERR_ALLOC;
	}

	for (pwd = pathname + i; *pwd; pwd = pathname + i) {
		len = tok_strlen(pwd, '/');

		/* if not a directory, it must be at the end of path. */
		if (!(curr.mode & FT_DIR))
			break;

		for (offset = 0; offset < curr.size; offset += dir.rec_len) {
			read_data_block(&curr, offset, &dir, dir_size, dev);

			if ((name = kmalloc(dir.name_len+1)) == NULL)
				return (char *)-ERR_ALLOC;

			read_data_block(&curr, offset + dir_size, name,
					dir.name_len+1, dev);

			if (!strncmp(name, pwd, len)) {
				kfree(name);
				break;
			}

			kfree(name);
		}

		/* no such path exist if it reaches at the end of dir list */
		if (offset >= curr.size)
			break;

		/* or move on */
		curr.addr = dir.inode;
		if (read_inode(&curr, dev)) {
			debug(MSG_DEBUG, "wrong inode");
		}

		i += len;
	}

	memcpy(inode, &curr, sizeof(struct embed_inode));

	/* if pwd is null after all, the inode you're looking for is found.
	 * or remained path indicates broken, no such directory. return the
	 * last one found. */

	return pathname + i;
}

static int create_file(const char *filename, mode_t mode,
		struct embed_inode *parent, const struct device *dev)
{
	struct embed_dir dir;
	unsigned int inode_new;

	/* check if the same filename exists */

	if (!(inode_new = make_node(mode, dev)))
		return -ERR_RANGE;

	dir.inode = inode_new;
	dir.name_len = strlen(filename) + 1;
	dir.rec_len = sizeof(struct embed_dir) + dir.name_len - sizeof(char *);
	dir.type = GET_FILE_TYPE(mode);
	dir.name = (char *)filename;

	write_data_block(parent, &dir,
			sizeof(struct embed_dir) - sizeof(char *), dev);
	write_data_block(parent, filename, dir.name_len, dev);

	return 0;
}

static size_t embed_read_core(struct file *file, void *buf, size_t len)
{
	int retval;
	struct embed_inode inode;

	inode.addr = file->inode->addr;
	if (read_inode(&inode, file->inode->sb->dev))
		return 0;

	retval = read_data_block(&inode, file->offset, buf, len,
			file->inode->sb->dev);

	if (retval > 0)
		file->offset += retval;

	return retval;
}

static size_t embed_read(struct file *file, void *buf, size_t len)
{
	struct task *parent;
	int tid;

	parent = current;
	tid = clone(TASK_HANDLER | TASK_KERNEL | STACK_SHARED, &init);

	if (tid == 0) { /* parent */
		set_task_state(current, TASK_WAITING);
		resched();
		return 0;
	} else if (tid < 0) { /* error */
		debug(MSG_SYSTEM, "failed cloning");
		return -ERR_RETRY;
	}

	__set_retval(parent, embed_read_core(file, buf, len));

	sum_curr_stat(parent);

	if (get_task_state(parent)) {
		set_task_state(parent, TASK_RUNNING);
		runqueue_add(parent);
	}

	sys_kill((unsigned int)current);
	freeze(); /* never reaches here */

	return -ERR_UNDEF;
}

static size_t embed_write_core(struct file *file, void *buf, size_t len)
{
	int retval;
	size_t size;
	struct embed_inode inode;

	inode.addr = file->inode->addr;
	if (read_inode(&inode, file->inode->sb->dev))
		return 0;

	size = inode.size;
	inode.size = file->offset;
	retval = write_data_block(&inode, buf, len, file->inode->sb->dev);

	if (retval > 0)
		file->offset += retval;

	if (file->offset > inode.size)
		debug(MSG_DEBUG, "file offset exceeds file size");

	if (inode.size > size) {
		unsigned int irqflag;
		spin_lock_irqsave(file->inode->lock, irqflag);
		file->inode->size = inode.size;
		spin_unlock_irqrestore(file->inode->lock, irqflag);
	}

	return retval;
}

static size_t embed_write(struct file *file, void *buf, size_t len)
{
	struct task *parent;
	int tid;

	parent = current;
	tid = clone(TASK_HANDLER | TASK_KERNEL | STACK_SHARED, &init);

	if (tid == 0) {
		set_task_state(current, TASK_WAITING);
		resched();
		return 0;
	} else if (tid < 0) {
		debug(MSG_SYSTEM, "failed cloning");
		return -ERR_RETRY;
	}

	__set_retval(parent, embed_write_core(file, buf, len));

	sum_curr_stat(parent);

	if (get_task_state(parent)) {
		set_task_state(parent, TASK_RUNNING);
		runqueue_add(parent);
	}

	sys_kill((unsigned int)current);
	freeze(); /* never reaches here */

	return -ERR_UNDEF;
}

static int embed_open(struct inode *inode, struct file *file)
{
	file->offset = 0;
	file->count = 1;
	file->op = inode->fop;
	file->inode = inode;
	lock_init(&file->lock);

	return 0;
}

static int embed_seek(struct file *file, unsigned int offset, int whence)
{
	switch (whence) {
	case SEEK_SET:
		file->offset = offset;
		break;
	case SEEK_CUR:
		if (file->offset + offset < file->offset)
			return -ERR_RANGE;

		file->offset += offset;
		break;
	case SEEK_END:
		if ((file->inode->size - offset) < 0)
			return -ERR_RANGE;

		file->offset = file->inode->size - offset;
		break;
	}

	return 0;
}

static int embed_lookup(struct inode *inode, const char *pathname)
{
	struct embed_inode embed_inode;
	const char *s;

	if ((int)(s = lookup(&embed_inode, pathname, inode->sb->dev)) < 0)
		return (int)s;

	if (*s)
		return -ERR_PATH;

	inode->addr = embed_inode.addr;
	inode->mode = embed_inode.mode;
	inode->size = embed_inode.size;

	return 0;
}

static int embed_create(struct inode *inode, const char *pathname, mode_t mode)
{
	struct embed_inode embed_inode;
	const char *s;

	if ((int)(s = lookup(&embed_inode, pathname, inode->sb->dev)) < 0)
		return (int)s;

	if (!*s || toknum(s, "/"))
		return -ERR_DUP;

	if (create_file(s, mode, &embed_inode, inode->sb->dev))
		return -ERR_CREATE;

	return embed_lookup(inode, pathname);
}

static int embed_close(struct file *file)
{
	__sync(file->inode->sb->dev);

	return 0;
}

static struct inode_operations iops = {
	.lookup = embed_lookup,
	.create = embed_create,
};

static struct file_operations fops = {
	.open  = embed_open,
	.read  = embed_read,
	.write = embed_write,
	.close = embed_close,
	.seek  = embed_seek,
};

static void embed_read_inode(struct inode *inode)
{
	struct embed_inode embed_inode;

	embed_inode.addr = inode->addr;

	if (read_inode(&embed_inode, inode->sb->dev) ||
			(inode->addr != embed_inode.addr)) {
		inode->iop = NULL;
		inode->fop = NULL;
		return;
	}

	inode->mode = embed_inode.mode;
	inode->size = embed_inode.size;
	inode->iop = &iops;
	inode->fop = &fops;
}

static int build_file_system(const struct device *dev)
{
	unsigned int disk_size, nr_inodes;

	disk_size = dev->block_size * dev->nr_blocks;
	nr_inodes = disk_size / INODE_TABLE_SIZE(sizeof(struct embed_inode));

	debug(MSG_SYSTEM, "disk size %d", disk_size);

	unsigned int nr_blocks, nr_data_bitmap;
	unsigned int inode_table;

	nr_blocks = disk_size / BLOCK_SIZE;
	nr_data_bitmap = nr_blocks / 8;
	nr_data_bitmap += (nr_blocks % 8)? 1 : 0;
	inode_table = nr_data_bitmap / BLOCK_SIZE + 1;
	inode_table += 2;

	unsigned int nr_inode_table;
	unsigned int data_block;

	nr_inode_table = ALIGN_BLOCK(sizeof(struct embed_inode) *
			nr_inodes,BLOCK_SIZE) / BLOCK_SIZE + 1;
	data_block = inode_table + nr_inode_table;

	struct embed_superblock sb;

	sb.block_size = BLOCK_SIZE;
	sb.blocks_count = nr_blocks;
	sb.inodes_count = nr_inodes;
	sb.free_inodes_count = nr_inodes;
	sb.free_blocks_count = nr_blocks - data_block;
	sb.inode_table = inode_table;
	sb.data_block = data_block;
	sb.first_block = dev->base_addr;
	sb.magic = MAGIC;

	write_superblock(&sb, dev);

	char *buf, *data_bitmap;
	unsigned int allocated, i;

	if ((buf = kmalloc(BLOCK_SIZE)) == NULL)
		return -ERR_ALLOC;
	if ((data_bitmap = kmalloc(nr_data_bitmap)) == NULL)
		return -ERR_ALLOC;

	/* initialize bitmaps */
	allocated = data_block / 8;
	memset(data_bitmap, 0xff, allocated);
	memset(data_bitmap + allocated, 0, nr_data_bitmap - allocated);
	if (nr_blocks % 8) data_bitmap[nr_data_bitmap-1] |= ~(nr_blocks % 8);
	data_bitmap[allocated] |= data_block % 8;
	memset(buf, 0xff, BLOCK_SIZE);
	write_block(inode_table-1, buf, BLOCK_SIZE, dev);
	memset(buf, 0, nr_inodes / 8);
	buf[nr_inodes / 8] &= ~(nr_inodes % 8);
	write_inode_bitmap(buf, dev);
	for (i = 0; i < (nr_data_bitmap / BLOCK_SIZE); i++)
		write_block(D_BMAP_BLK + i, data_bitmap + (i * BLOCK_SIZE),
				BLOCK_SIZE, dev);
	if (nr_data_bitmap % BLOCK_SIZE)
		write_block(D_BMAP_BLK + i, data_bitmap + (i * BLOCK_SIZE),
				nr_data_bitmap % BLOCK_SIZE, dev);

	debug(MSG_DEBUG, "nr_data_bitmap %d", nr_data_bitmap);

	kfree(buf);
	kfree(data_bitmap);

	/* make the root node. root inode is always 0. */
	if (make_node(FT_ROOT, dev) != 0) {
		debug(MSG_DEBUG, "wrong root inode");
		return -ERR_UNDEF;
	}

	struct embed_inode root_inode;
	root_inode.addr = 0;
	read_inode(&root_inode, dev);
	create_file("dev", FT_DIR, &root_inode, dev);

	debug(MSG_SYSTEM, "built embed file system:\n"
			"block_size %d\n"
			"blocks_count %d\n"
			"free_inodes_count %d\n"
			"free_blocks_count %d\n"
			"inode_table %d\n"
			"data_block %d\n"
			"first_block %x\n"
			"magic %x\n",
			sb.block_size,
			sb.blocks_count,
			sb.free_inodes_count,
			sb.free_blocks_count,
			sb.inode_table,
			sb.data_block,
			sb.first_block,
			sb.magic);
	return 0;
}

int embedfs_mount(const struct device *dev)
{
	unsigned int end;
	end = dev->block_size * dev->nr_blocks + dev->base_addr - 1;
	debug(MSG_SYSTEM, "embedfs addr %08x - %08x", dev->base_addr, end);

	struct embed_superblock sb;

	read_superblock(&sb, dev);

	if (sb.magic != MAGIC)
		build_file_system(dev);

	read_superblock(&sb, dev);

	if (sb.magic != MAGIC)
		goto err;
	if (sb.block_size != BLOCK_SIZE)
		goto err;
	if (sb.first_block != dev->base_addr)
		goto err;

	return 0;
err:
	debug(MSG_SYSTEM, "can't build root file system");
	return -ERR_UNDEF;
}

static struct super_operations super_ops = {
	.read_inode = embed_read_inode,
	.mount = embedfs_mount,
};

static int read_super(struct superblock *sb, const struct device *dev)
{
	/* no need to read from disk because the root inode is always zero */
	sb->root_inode = 0;
	sb->op = &super_ops;

	return 0;
}

void embedfs_register()
{
	struct file_system_type *fs;

	fs = kmalloc(sizeof(struct file_system_type));
	fs->read_super = read_super;

	add_file_system(fs, "embedfs");
}
