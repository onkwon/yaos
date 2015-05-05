#include <syscall.h>
#include <kernel/device.h>

int sys_open(int id, int mode)
{
	struct device_t *dev = getdev(id);

	if (!dev || !dev->ops->open)
		return -SYSCALL_UNDEF;

	return dev->ops->open(id, mode);
}

int sys_read(int id, void *buf, size_t size)
{
	struct device_t *dev = getdev(id);

	if (!dev || !dev->ops->read)
		return -SYSCALL_UNDEF;

	return dev->ops->read(id, buf, size);
}

int sys_write(int id, void *buf, size_t size)
{
	struct device_t *dev = getdev(id);

	if (!dev || !dev->ops->write)
		return -SYSCALL_UNDEF;

	return dev->ops->write(id, buf, size);
}

int sys_close(int id)
{
	struct device_t *dev = getdev(id);

	if (!dev || !dev->ops->write)
		return -SYSCALL_UNDEF;

	return dev->ops->close(id);
}

int sys_reserved()
{
	return -SYSCALL_UNDEF;
}

int sys_test(int a, int b, int c)
{
	return a + b + c;
}

extern int sys_schedule();

void *syscall_table[] = {
	sys_reserved,		/* 0 */
	sys_schedule,
	sys_test,
	sys_open,
	sys_read,
	sys_write,		/* 5 */
	sys_close,
};
