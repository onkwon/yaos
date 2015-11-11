#include <kernel/syscall.h>
#include <kernel/device.h>
#include <kernel/page.h>
#include <error.h>
#include <stdlib.h>
#include <io.h>

int sys_open_core(char *filename, int mode, void *opt)
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
		goto errout;
	}

	if (new->iop->lookup(new, filename + sb->pathname_len)) {
		if (!(mode & O_CREATE)) {
			err = -ERR_PATH;
			goto errout;
		}

		if (!new->iop->create || new->iop->create(new,
					filename + sb->pathname_len, FT_FILE)) {
			err = -ERR_CREATE;
			goto errout;
		}
	}

	if ((inode = iget(new->sb, new->addr)) == NULL) {
		iput(new);
		inode = new;
		inode->count = 0;
		lock_init(&inode->lock);
	} else {
		kfree(new);
	}

	spin_lock(inode->lock);
	inode->count++;
	spin_unlock(inode->lock);
	file.flags = mode;
	file.option = opt;
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

errout:
	kfree(new);
	return err;
}

int sys_open(char *filename, int mode, void *opt)
{
	struct task *parent;
	int tid;

	parent = current;
	tid = clone(TASK_HANDLER | TASK_KERNEL | STACK_SHARED, &init);

	if (tid == 0) { /* parent */
		/* it goes TASK_WAITING state as soon as exiting from system
		 * call to wait for its child's job to be done that returns the
		 * result. */
		set_task_state(current, TASK_WAITING);
		resched();
		return 0;
	} else if (tid < 0) { /* error */
		/* use errno */
		debug(MSG_DEBUG, "failed cloning");
		return -ERR_RETRY;
	}

	/* child takes place from here turning to kernel task,
	 * nomore in handler mode */
	__set_retval(parent, sys_open_core(filename, mode, opt));

	sum_curr_stat(parent);

	if (get_task_state(parent)) {
		set_task_state(parent, TASK_RUNNING);
		runqueue_add(parent);
	}

	sys_kill((unsigned int)current);
	freeze(); /* never reaches here */

	return -ERR_UNDEF;
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
#include <kernel/timer.h> /* sys_timer_create() */

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
	sys_timer_create,
	//sys_create,
	//sys_mkdir,		/* 15 */
};

#define FORWARD(addr)		((int *)(addr)++)
#define BACKWARD(addr)		((int *)(addr)--)
#define getarg(args, type)	(*(type *)FORWARD(args))

int open(char *filename, ...)
{
	void *opt;
	int *args, mode;
	char *base;

	args = (int *)&filename;
	base = getarg(args, char *);
	mode = getarg(args, int);
	opt  = getarg(args, void *);

#ifdef CONFIG_SYSCALL
	return syscall(SYSCALL_OPEN, base, mode, opt);
#else
	sys_open(base, mode, opt);
#endif
}
