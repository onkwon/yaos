#include <kernel/module.h>
#include <kernel/gpio.h>
#include <error.h>

#define GPIO_PIN_INDEX		50

static size_t led_read(struct file *file, void *buf, size_t len)
{
	unsigned int *p = (unsigned int *)buf;

	*p = gpio_get(GPIO_PIN_INDEX);

	return 1;
}

static size_t led_write(struct file *file, void *buf, size_t len)
{
	unsigned int *p = (unsigned int *)buf;

	gpio_put(GPIO_PIN_INDEX, *p);

	return 1;
}

static struct file_operations ops = {
	.open  = NULL,
	.read  = led_read,
	.write = led_write,
	.close = NULL,
	.seek  = NULL,
};

#include <kernel/init.h>

static void __init led_init()
{
	struct device *dev;

	if ((dev = mkdev(0, 0, &ops, "led")))
		gpio_init(GPIO_PIN_INDEX, GPIO_MODE_OUTPUT);
}
MODULE_INIT(led_init);
