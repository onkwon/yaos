#include <kernel/syscall.h>
#include <kernel/device.h>
#include <kernel/page.h>
#include <error.h>
#include <stdlib.h>

int sys_open(char *filename, int mode)
{
	struct superblock *sb;
	struct inode *inode, *new;
	struct file file;
	struct file_operations *ops;
	int err;

	if ((sb = search_super(filename)) == NULL)
		return -ERR_PATH;

	if ((new = kmalloc(sizeof(struct inode))) == NULL)
		return -ERR_ALLOC;

	new->addr = sb->root_inode;
	new->sb = sb;
	sb->op->read_inode(new);

	if (new->iop == NULL) /* probably due to failuer of memory allocation */
		return -ERR_RETRY;

	if ((ops = kmalloc(sizeof(struct file_operations))) == NULL) {
		err = -ERR_ALLOC;
		goto outerr;
	}

	if (new->iop->lookup(new, filename + sb->pathname_len)) {
		if (!(mode & O_CREATE)) {
			err = -ERR_PATH;
			goto outerr;
		}

		if (!new->iop->create || new->iop->create(new,
					filename + sb->pathname_len, FT_FILE)) {
			err = -ERR_CREATE;
			goto outerr;
		}
	}

	if ((inode = iget(new->sb, new->addr)) == NULL) {
		iput(new);
		inode = new;
		inode->count = 0;
		INIT_LOCK(inode->lock);
	} else {
		kfree(new);
	}

	spin_lock(inode->lock);
	inode->count++;
	spin_unlock(inode->lock);
	file.flags = mode;
	inode->fop->open(inode, &file);

	memcpy(ops, file.op, sizeof(struct file_operations));
	file.op = ops;

	if (GET_FILE_TYPE(inode->mode) == FT_DEV) {
		memcpy(file.op, getdev(inode->dev)->op,
				sizeof(struct file_operations));

		if (file.op->open)
			file.op->open(inode, &file);
	}

	return mkfile(&file);

outerr:
	kfree(new);
	return err;
}

int sys_read(int fd, void *buf, size_t len)
{
	struct file *file = getfile(fd);

	if (!file || !file->op->read)
		return -ERR_UNDEF;
	if (!(file->flags & O_RDONLY))
		return -ERR_PERM;

	return file->op->read(file, buf, len);
}

int sys_write(int fd, void *buf, size_t len)
{
	struct file *file = getfile(fd);

	if (!file || !file->op->write)
		return -ERR_UNDEF;
	if (!(file->flags & O_WRONLY))
		return -ERR_PERM;

	return file->op->write(file, buf, len);
}

int sys_close(int fd)
{
	struct file *file = getfile(fd);

	if (!file)
		return -ERR_UNDEF;

	if (file->op->close)
		file->op->close(file);

	remove_file(file);

	return 0;
}

int sys_seek(int fd, unsigned int offset, int whence)
{
	struct file *file = getfile(fd);

	if (!file || !file->op->seek)
		return -ERR_UNDEF;

	return file->op->seek(file, offset, whence);
}

int sys_reserved()
{
	return -ERR_UNDEF;
}

int sys_test(int a, int b, int c)
{
	return a + b + c;
}

#include <kernel/sched.h>

void *syscall_table[] = {
	sys_reserved,		/* 0 */
	sys_schedule,
	sys_test,
	sys_open,
	sys_read,
	sys_write,		/* 5 */
	sys_close,
	sys_brk,
	sys_yield,
	sys_mknod,
	sys_seek,		/* 10 */
	sys_kill,
	sys_fork,
	//sys_create,
	//sys_mkdir,
};
