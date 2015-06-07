#include <kernel/syscall.h>
#include <kernel/device.h>
#include <kernel/page.h>
#include <error.h>

int sys_open(char *filename, int mode)
{
	struct superblock *sb;
	struct inode *inode;
	struct file file;

	if ((sb = search_super(filename)) == NULL)
		return -ERR_PATH;

	if ((inode = kmalloc(sizeof(struct inode))) == NULL)
		return -ERR_ALLOC;

	inode->addr = sb->root_inode;
	inode->sb = sb;
	sb->op->read_inode(inode);

	if (inode->iop->lookup(inode, filename + sb->pathname_len))
		return -ERR_PATH;

	file.flags = mode;
	inode->fop->open(inode, &file);

	if (GET_FILE_TYPE(inode->mode) == FT_DEV) {
		file.op = getdev(inode->dev)->op;
		if (file.op->open)
			file.op->open(inode, &file);
	}

	return mkfile(&file);
}

int sys_read(int fd, void *buf, size_t size)
{
	struct file *file = getfile(fd);

	if (!file || !file->op->read)
		return -ERR_UNDEF;

	return file->op->read(file, buf, size);
}

int sys_write(int fd, void *buf, size_t size)
{
	struct file *file = getfile(fd);

	if (!file || !file->op->write)
		return -ERR_UNDEF;

	return file->op->write(file, buf, size);
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
	//sys_create,
	//sys_mkdir,
};
