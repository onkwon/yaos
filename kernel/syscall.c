#include <kernel/syscall.h>
#include <kernel/device.h>
#include <kernel/page.h>
#include <error.h>
#include <stdlib.h>
#include <io.h>

void syscall_delegate_return(struct task *task, int ret)
{
	unsigned int irqflag;

	spin_lock_irqsave(&task->lock, irqflag);

	__set_retval(task, ret);
	sum_curr_stat(task);

	if (get_task_state(task)) {
		set_task_state(task, TASK_RUNNING);
		runqueue_add_core(task);
	}

	spin_unlock_irqrestore(&task->lock, irqflag);
}

void syscall_delegate(struct task *org, struct task *delegate)
{
	spin_lock(&org->lock);
	set_task_state(org, TASK_WAITING);
	spin_unlock(&org->lock);

	spin_lock(&delegate->lock);
	set_task_state(delegate, TASK_RUNNING);
	runqueue_add_core(delegate);
	spin_unlock(&delegate->lock);

	resched();
}

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
	INIT_MUTEX(new->lock);
	sb->op->read_inode(new);

	if (new->iop == NULL) /* probably due to failure of memory allocation */
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
		ilink(new);
		inode = new;
		inode->refcount = 0;
		INIT_MUTEX(inode->lock);
	} else {
		kfree(new);
	}

	mutex_lock(&inode->lock);
	inode->refcount++;
	mutex_unlock(&inode->lock);

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
		spin_lock(&current->lock);
		set_task_state(current, TASK_WAITING);
		spin_unlock(&current->lock);
		resched();
		return 0;
	} else if (tid < 0) { /* error */
		/* use errno */
		error("failed cloning");
		return -ERR_RETRY;
	}

	unsigned int irqflag;
	int ret;

	/* child takes place from here turning to kernel task,
	 * nomore in handler mode */
	ret = sys_open_core(filename, mode, option);

	spin_lock_irqsave(&parent->lock, irqflag);

	__set_retval(parent, ret);
	sum_curr_stat(parent);

	if (get_task_state(parent)) {
		set_task_state(parent, TASK_RUNNING);
		runqueue_add_core(parent);
	}

	spin_unlock_irqrestore(&parent->lock, irqflag);

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

	if (file->op->close &&
			(file->op->close(file) == SYSCALL_DEFERRED_WORK))
		return 0;

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

int sys_ioctl(int fd, int request, void *data)
{
	struct file *file = getfile(fd);

	if (!file || !file->op->ioctl)
		return -ERR_UNDEF;

	return file->op->ioctl(file, request, data);
}

int sys_remove_core(const char *pathname)
{
	struct superblock *sb;
	struct inode *inode, *found;
	int err = 0;

	pathname = current->args;

	if ((sb = search_super(pathname)) == NULL) {
		err = -ERR_PATH;
		goto out;
	}

	if ((inode = kmalloc(sizeof(*inode))) == NULL) {
		err = -ERR_ALLOC;
		goto out;
	}

	inode->addr = sb->root_inode;
	inode->sb = sb;
	INIT_MUTEX(inode->lock);
	sb->op->read_inode(inode);

	if (inode->iop == NULL) { /* probably due to failure of memory allocation */
		err = -ERR_RETRY;
		goto out_free_inode;
	}

	if (inode->iop->lookup(inode, pathname + sb->pathname_len)) {
		err = -ERR_PATH;
		goto out_free_inode;
	}

	if ((found = iget(inode->sb, inode->addr)) == NULL)
		found = inode;

	mutex_lock(&found->lock);

	if ((err = found->iop->delete(found, pathname)) || found == inode)
		goto out_free_inode;

	while ((volatile typeof(found->refcount))found->refcount) {
		mutex_unlock(&found->lock);
		resched();
		mutex_lock(&found->lock);
	}

	iunlink(found);
	kfree(found);

out_free_inode:
	kfree(inode);
out:
	syscall_delegate_return(current->parent, err);

	return err;
}

int sys_remove(const char *pathname)
{
	struct task *thread;

	if ((thread = make(TASK_HANDLER | STACK_SHARED, sys_remove_core,
					current)) == NULL)
		return -ERR_ALLOC;

	thread->args = (void *)pathname;
	syscall_delegate(current, thread);

	return 0;
}

#include <asm/power.h>

int sys_shutdown(int option)
{
	if (!(get_task_type(current) & TASK_PRIVILEGED))
		return -ERR_PERM;

	/* FIXME: Clone here otherwise it causes a fault since sync() needs
	 * a system call */
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
	sys_remove,
	//sys_create,
	//sys_mkdir,
};

#define getarg(args, type)	({ \
	args = (char *)ALIGN_BLOCK(args, sizeof(type)); \
	args = (char *)((unsigned int)(args) + sizeof(type)); \
	*(type *)((unsigned int)(args) - sizeof(type)); \
})

int __open(const char *filename, ...)
{
	const char *base, *path;
	int mode;
	void *opt;

	base = (const char *)&filename;
	path = getarg(base, char *);
	mode = getarg(base, int);
	opt = getarg(base, void *);

#ifdef CONFIG_SYSCALL
	return syscall(SYSCALL_OPEN, path, mode, opt);
#else
	sys_open(path, mode, opt);
#endif
}

int __open2(const char *filename, int mode)
{
#ifdef CONFIG_SYSCALL
	return syscall(SYSCALL_OPEN, filename, mode, 0);
#else
	sys_open(filename, mode, NULL);
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
	int request;
	void *data;
	char *base;

	base    = (char *)&fd;
	fd      = getarg(base, int);
	request = getarg(base, int);
	data    = getarg(base, void *);

#ifdef CONFIG_SYSCALL
	return syscall(SYSCALL_IOCTL, fd, request, data);
#else
	sys_ioctl(fd, request, data);
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

bool has_event(int fd)
{
	int byte = 0;
	ioctl(fd, C_EVENT, &byte);
	return (bool)byte;
}
