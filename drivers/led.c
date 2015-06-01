#include <kernel/module.h>
#include <kernel/gpio.h>
#include <error.h>

#define GPIO_PIN_INDEX		50

int led_id;

static size_t led_read(unsigned int id, void *buf, size_t len)
{
	unsigned int *p = (unsigned int *)buf;

	*p = gpio_get(GPIO_PIN_INDEX);

	return 1;
}

static size_t led_write(unsigned int id, void *buf, size_t len)
{
	unsigned int *p = (unsigned int *)buf;

	gpio_put(GPIO_PIN_INDEX, *p);

	return 1;
}

static struct dev_interface_t ops = {
	.open  = NULL,
	.read  = led_read,
	.write = led_write,
	.close = NULL,
};

#include <kernel/init.h>

static int __init led_init()
{
	led_id = mkdev();

	if (register_dev(led_id, &ops, "led"))
		return -ERR_RANGE;

	gpio_init(GPIO_PIN_INDEX, GPIO_MODE_OUTPUT);

	return 0;
}
MODULE_INIT(led_init);
