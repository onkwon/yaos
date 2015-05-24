#include <kernel/syscall.h>
#include <kernel/module.h>
#include <asm/context.h>
#include <error.h>

int sys_open(int id, int mode)
{
	struct device_t *dev = getdev(id);

	if (!dev || !dev->ops->open)
		return -ERR_UNDEF;

	return dev->ops->open(id, mode);
}

int sys_read(int id, void *buf, size_t size)
{
	struct device_t *dev = getdev(id);

	if (!dev || !dev->ops->read)
		return -ERR_UNDEF;

	return dev->ops->read(id, buf, size);
}

int sys_write(int id, void *buf, size_t size)
{
	struct device_t *dev = getdev(id);

	if (!dev || !dev->ops->write)
		return -ERR_UNDEF;

	return dev->ops->write(id, buf, size);
}

int sys_close(int id)
{
	struct device_t *dev = getdev(id);

	if (!dev || !dev->ops->write)
		return -ERR_UNDEF;

	return dev->ops->close(id);
}

int sys_reserved()
{
	return -ERR_UNDEF;
}

int sys_test(int a, int b, int c)
{
	return a + b + c;
}

#include <kernel/page.h>

void *sys_brk(unsigned long size)
{
	return kmalloc(size);
}

void *syscall_table[] = {
	sys_reserved,		/* 0 */
	sys_schedule,
	sys_test,
	sys_open,
	sys_read,
	sys_write,		/* 5 */
	sys_close,
	sys_brk,
};
