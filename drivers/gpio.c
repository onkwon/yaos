#include <error.h>
#include <kernel/module.h>
#include <kernel/gpio.h>
#include <asm/pinmap.h>

/* TODO: support interrupt mode
 *   1. register the interrupt service routine
 *   2. call a user defined function passed by ioctl() in ISR
 *   3. provide edge configuration through ioctl(),
 *      GPIO_INT_FALLING and GPIO_INT_RISING */

static unsigned int major;

static size_t gpio_read(struct file *file, void *buf, size_t len)
{
	unsigned int *p = (unsigned int *)buf;

	*p = gpio_get(MINOR(file->inode->dev));

	return 1;
}

static size_t gpio_write(struct file *file, void *buf, size_t len)
{
	unsigned int *p = (unsigned int *)buf;

	gpio_put(MINOR(file->inode->dev), *p);

	return 1;
}

static int gpio_open(struct inode *inode, struct file *file)
{
	struct device *dev = getdev(file->inode->dev);
	int ret = 0;

	if (dev == NULL)
		return -ERR_UNDEF;

	spin_lock(&dev->mutex.counter);

	if (dev->refcount == 0) {
		if (!(get_task_flags(current->parent) & TASK_PRIVILEGED)) {
			ret = -ERR_PERM;
			goto out_unlock;
		}

		/* TODO: check if the pin is already being used */
		switch (file->flags & O_RDWR) {
		case O_RDONLY:
			gpio_init(MINOR(file->inode->dev),
					GPIO_MODE_INPUT | GPIO_CONF_PULL_UP);
			break;
		case O_WRONLY:
			gpio_init(MINOR(file->inode->dev), GPIO_MODE_OUTPUT);
			break;
		default:
			ret = -ERR_UNDEF;
			goto out_unlock;
			break;
		}
	}

	dev->refcount++;

out_unlock:
	spin_unlock(&dev->mutex.counter);

	return ret;
}

static int gpio_close(struct file *file)
{
	struct device *dev = getdev(file->inode->dev);

	if (dev == NULL)
		return -ERR_UNDEF;

	spin_lock(&dev->mutex.counter);

	if (--dev->refcount == 0) {
		if (!(get_task_flags(current) & TASK_PRIVILEGED))
			return -ERR_PERM;

		//gpio_reset(MINOR(file->inode->dev));
	}

	spin_unlock(&dev->mutex.counter);

	return 0;
}

static int gpio_ioctl(struct file *file, int request, void *data)
{
	if (!(get_task_flags(current) & TASK_PRIVILEGED))
		return -ERR_PERM;

	return 0;
}

static struct file_operations ops = {
	.open  = gpio_open,
	.read  = gpio_read,
	.write = gpio_write,
	.close = gpio_close,
	.seek  = NULL,
	.ioctl = gpio_ioctl,
};

void register_gpio(const char *name, int minor)
{
	macro_register_device(name, major, minor, &ops);
}
