#include <kernel/syscall.h>
#include <kernel/device.h>
#include <kernel/page.h>
#include <error.h>
#include <stdlib.h>
#include <io.h>

int sys_open_core(char *filename, int mode, void *option)
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
		goto out_free;
	}

	if (new->iop->lookup(new, filename + sb->pathname_len)) {
		if (!(mode & O_CREATE)) {
			err = -ERR_PATH;
			goto out_free;
		}

		if (!new->iop->create || new->iop->create(new,
					filename + sb->pathname_len, FT_FILE)) {
			err = -ERR_CREATE;
			goto out_free;
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

	spin_lock(&inode->lock);
	inode->count++;
	spin_unlock(&inode->lock);

	if (GET_FILE_TYPE(inode->mode) == FT_DEV)
		memcpy(ops, getdev(inode->dev)->op,
				sizeof(struct file_operations));
	else
		memcpy(ops, inode->fop, sizeof(struct file_operations));

	file.flags  = mode;
	file.option = option;
	file.offset = 0;
	file.inode  = inode;
	file.op     = ops;

	if (file.op->open)
		file.op->open(inode, &file);

	return mkfile(&file);

out_free:
	kfree(new);

	return err;
}

int sys_open(char *filename, int mode, void *option)
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
		debug(MSG_SYSTEM, "failed cloning");
		return -ERR_RETRY;
	}

	/* child takes place from here turning to kernel task,
	 * nomore in handler mode */
	__set_retval(parent, sys_open_core(filename, mode, option));

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

	rmfile(file);

	return 0;
}

int sys_seek(int fd, unsigned int offset, int whence)
{
	struct file *file = getfile(fd);

	if (!file || !file->op->seek)
		return -ERR_UNDEF;

	return file->op->seek(file, offset, whence);
}

int sys_ioctl(int fd, unsigned int request, void *args)
{
	struct file *file = getfile(fd);

	if (!file || !file->op->ioctl)
		return -ERR_UNDEF;

	return file->op->ioctl(file, request, args);
}

#include <asm/power.h>

int sys_shutdown(int option)
{
	if (!(get_task_type(current) & TASK_PRIVILEGED))
		return -ERR_PERM;

	switch (option) {
	case 1: /* shutdown */
		break;
	case 2: /* reboot */
		/* sync() */
		__reboot();
	default:
		break;
	}

	return 0;
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
	sys_shutdown,
	sys_ioctl,		/* 15 */
	//sys_create,
	//sys_mkdir,
};

#define FORWARD(addr)		((int *)(addr)++)
#define BACKWARD(addr)		((int *)(addr)--)
#define getarg(args, type)	(*(type *)FORWARD(args))

int open(char *filename, ...)
{
	void *option;
	int *base, mode;
	char *fmt;

	base   = (int *)&filename;
	fmt    = getarg(base, char *);
	mode   = getarg(base, int);
	option = getarg(base, void *);

#ifdef CONFIG_SYSCALL
	return syscall(SYSCALL_OPEN, fmt, mode, option);
#else
	sys_open(fmt, mode, option);
#endif
}

int close(int fd)
{
#ifdef CONFIG_SYSCALL
	return syscall(SYSCALL_CLOSE, fd);
#else
	sys_close(fd);
#endif
}

int ioctl(int fd, ...)
{
	void *args;
	unsigned int request;
	int *base;

	base    = (int *)&fd;
	fd      = getarg(base, int);
	request = getarg(base, unsigned int);
	args    = getarg(base, void *);

#ifdef CONFIG_SYSCALL
	return syscall(SYSCALL_IOCTL, fd, request, args);
#else
	sys_ioctl(fd, request, args);
#endif
}

int shutdown(int option)
{
#ifdef CONFIG_SYSCALL
	return syscall(SYSCALL_SHUTDOWN, option);
#else
	sys_shutdown(option);

	return 0;
#endif
}
