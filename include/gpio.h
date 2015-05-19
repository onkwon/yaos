#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_MODE_INPUT			0x0000
#define GPIO_MODE_OUTPUT		0x0001	

#define GPIO_CONF_ANALOG		0x0002
#define GPIO_CONF_FLOAT			0x0004
#define GPIO_CONF_PULL			0x0008 /* PUSHPULL in case of output */
#define GPIO_CONF_GENERAL		0x0010
#define GPIO_CONF_OPENDRAIN		0x0020
#define GPIO_CONF_ALT			0x0040
#define GPIO_CONF_ALT_OPENDRAIN		0x0080

#define GPIO_INT_FALLING		0x0100
#define GPIO_INT_RISING			0x0200

int gpio_init(unsigned int n, unsigned int flags);

#include <asm/gpio.h>

#endif /* __GPIO_H__ */
