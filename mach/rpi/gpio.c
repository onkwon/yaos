#include <gpio.h>
#include <types.h>

static DEFINE_SPINLOCK(gpio_init_lock);

void gpio_pull(unsigned int pin, unsigned int mode)
{
	volatile unsigned int *reg;
	unsigned int i;
	struct gpio *gpio = (struct gpio *)GPIO_BASE;

	if (pin / 32)
		reg = &gpio->pudclk1;
	else
		reg = &gpio->pudclk0;

	pin = pin % 32;

	gpio->pud = mode; /* 0: off, 1: pull down, 2: pull up */
	for (i = 0; i < 150; i++) __nop(); /* wait 150 cycles for set-up time */
	*reg = 1 << pin;
	for (i = 0; i < 150; i++) __nop(); /* hold time */
	*reg = 0; /* clean */
}

unsigned int gpio_get(unsigned int index)
{
	volatile unsigned int *base;
	unsigned int port, pin;

	port = index / 32;
	pin  = index % 32;

	if (port)
		base = (volatile unsigned int *)(GPIO_BASE + 0x38);
	else
		base = (volatile unsigned int *)(GPIO_BASE + 0x34);

	return (*base & (1 << pin)) >> pin;
}

void gpio_put(unsigned int index, int v)
{
	if (v & 1)
		GPIO_SET(index);
	else
		GPIO_CLEAR(index);
}

int gpio_init(unsigned int index, unsigned int flags)
{
	unsigned int mode;
	int vector;
	unsigned int irqflag;

	struct gpio *gpio = (struct gpio *)GPIO_BASE;

	mode = 0;

	spin_lock_irqsave(gpio_init_lock, irqflag);

	if (flags & GPIO_MODE_OUTPUT) {
		mode |= GPIO_OUTPUT;
	} else if (flags & GPIO_MODE_INPUT) {
		mode |= GPIO_INPUT;
	}

	SET_GPIO_FS(index, mode);

	if (flags & GPIO_CONF_PULL) {
		gpio_pull(index, 2); /* pull up only */
	}

	if (flags & GPIO_INT_FALLING) {
		if (index / 32)
			gpio->f_edge1 |= 1 << (index % 32);
		else
			gpio->f_edge0 |= 1 << index;
	}

	if (flags & GPIO_INT_RISING) {
		if (index / 32)
			gpio->r_edge1 |= 1 << (index % 32);
		else
			gpio->r_edge0 |= 1 << index;
	}

	spin_unlock_irqrestore(gpio_init_lock, irqflag);

	struct interrupt_controller *intcntl
		= (struct interrupt_controller *)INTCNTL_BASE;
	intcntl->irq2_enable = 1 << (IRQ_GPIO0 % 32);

	vector = IRQ_GPIO0;
	return vector;
}

void gpio_close(unsigned int index)
{
}
