#ifndef __GPIO_H__
#define __GPIO_H__

enum gpio_mode {
	GPIO_MODE_INPUT		= 0x0000,
	GPIO_MODE_OUTPUT	= 0x0001,
	GPIO_MODE_ALT		= 0x0002,
	GPIO_MODE_ANALOG	= 0x0004,
	GPIO_MODE_MASK		= 0x0007,

	GPIO_CONF_PULLUP	= 0x0008,
	GPIO_CONF_PULLDOWN	= 0x0010,
	GPIO_CONF_OPENDRAIN	= 0x0020,
	GPIO_CONF_MASK		= 0x0038,

	GPIO_INT_FALLING	= 0x0100,
	GPIO_INT_RISING		= 0x0200,
	GPIO_INT_HIGH		= 0x0400,
	GPIO_INT_LOW		= 0x0800,
	GPIO_INT_MASK		= 0x0f00,

	GPIO_SPD_SLOW		= 0x1000,
	GPIO_SPD_MID		= 0x2000,
	GPIO_SPD_FAST		= 0x3000,
	GPIO_SPD_FASTEST	= 0x4000,
	GPIO_SPD_MASK		= 0xf000,
};

#define GPIO_ALT_SHIFT			16

#define gpio_altfunc(n)			\
	(GPIO_MODE_ALT | ((n) << GPIO_ALT_SHIFT))
#define gpio_altfunc_get(flags)		((flags) >> GPIO_ALT_SHIFT)

int gpio_init(unsigned int npin, unsigned int flags);
void gpio_fini(unsigned int npin);
void gpio_put(unsigned int npin, int v);
unsigned int gpio_get(unsigned int npin);

#include <asm/mach/gpio.h>

#endif /* __GPIO_H__ */
