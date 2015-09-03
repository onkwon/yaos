#include <gpio.h>
#include <types.h>

DEFINE_SPINLOCK(gpio_irq_lock);
DEFINE_SPINLOCK(gpio_init_lock);

unsigned int gpio_get(unsigned int index)
{
	return 0;
}

void gpio_put(unsigned int index, int v)
{
}

int gpio_init(unsigned int index, unsigned int flags)
{
	return 0;
}

void gpio_close(unsigned int index)
{
}
