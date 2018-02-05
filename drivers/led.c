/* This led driver is just for testing. */

#include <error.h>
#include <gpio.h>
#include <kernel/module.h>
#include <asm/pinmap.h>

static unsigned int major;

static size_t led_read(struct file *file, void *buf, size_t len)
{
	unsigned int *p = (unsigned int *)buf;

	*p = gpio_get((unsigned int)file->option);

	return 1;
}

static size_t led_write(struct file *file, void *buf, size_t len)
{
	unsigned int *p = (unsigned int *)buf;

	gpio_put((unsigned int)file->option, *p);

	return 1;
}

static int led_open(struct inode *inode, struct file *file)
{
	struct device *dev = getdev(file->inode->dev);
	int err = 0;

	if (dev == NULL)
		return -EFAULT;

	spin_lock(&dev->mutex.counter);

	if (dev->refcount == 0) {
		/* check permission here */
		gpio_init((unsigned int)file->option, GPIO_MODE_OUTPUT);
	}

	dev->refcount++;

	spin_unlock(&dev->mutex.counter);

	return err;
}

static struct file_operations ops = {
	.open  = led_open,
	.read  = led_read,
	.write = led_write,
	.close = NULL,
	.seek  = NULL,
	.ioctl = NULL,
};

void register_led(const char *name, int minor)
{
	macro_register_device(name, major, minor, &ops);
}
