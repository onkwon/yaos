/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

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
