#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_MODE_INPUT			0x0000
#define GPIO_MODE_OUTPUT		0x0001

/* output */
#define GPIO_CONF_OPENDRAIN		0x0002
#define GPIO_CONF_ALT			0x0004
/* input */
#define GPIO_CONF_ANALOG		0x0010
#define GPIO_CONF_FLOAT			0x0020
#define GPIO_CONF_PULL_UP		0x0040
#define GPIO_CONF_PULL_DOWN		0x0080
/* interrupt edge */
#define GPIO_INT_FALLING		0x0100
#define GPIO_INT_RISING			0x0200

int gpio_init(unsigned int index, unsigned int flags);
void gpio_close(unsigned int index);
void gpio_put(unsigned int index, int v);
unsigned int gpio_get(unsigned int index);

#include <asm/gpio.h>

#endif /* __GPIO_H__ */
