#include <kernel/module.h>
#include <kernel/gpio.h>
#include <error.h>

#define GPIO_PIN_INDEX		50

int led_id;
static DEFINE_SPINLOCK(lock);

static size_t led_read(int id, void *buf, size_t len)
{
	unsigned int *p = (unsigned int *)buf;

	spin_lock(lock);
	*p = gpio_get(GPIO_PIN_INDEX);
	spin_unlock(lock);

	return 1;
}

static size_t led_write(int id, void *buf, size_t len)
{
	unsigned int *p = (unsigned int *)buf;

	spin_lock(lock);
	gpio_put(GPIO_PIN_INDEX, *p);
	spin_unlock(lock);

	return 1;
}

static struct device_interface_t ops = {
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
