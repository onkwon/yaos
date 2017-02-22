#ifndef __GPIO_H__
#define __GPIO_H__

enum gpio_mode {
	GPIO_MODE_INPUT		= 0x0000,
	GPIO_MODE_OUTPUT	= 0x0001,
	GPIO_MODE_ALT		= 0x0002,
	GPIO_MODE_ANALOG	= 0x0004,

	GPIO_CONF_PULL_UP	= 0x0008,
	GPIO_CONF_PULL_DOWN	= 0x0010,
	GPIO_CONF_OPENDRAIN	= 0x0020,

	GPIO_INT_FALLING	= 0x0100,
	GPIO_INT_RISING		= 0x0200,
};

#define GPIO_ALT_SHIFT			16

#define gpio_altfunc(n)			\
	(GPIO_MODE_ALT | ((n) << GPIO_ALT_SHIFT))

int gpio_init(unsigned int index, unsigned int flags);
void gpio_reset(unsigned int index);
void gpio_put(unsigned int index, int v);
unsigned int gpio_get(unsigned int index);

#undef  INCPATH
#define INCPATH			<asm/mach-MACHINE/gpio.h>
#include INCPATH

#endif /* __GPIO_H__ */
