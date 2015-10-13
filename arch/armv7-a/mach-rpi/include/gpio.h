#ifndef __RPI_GPIO_H__
#define __RPI_GPIO_H__

#include <io.h>
#include <kernel/lock.h>

#define GPIO_INPUT		0
#define GPIO_OUTPUT		1
#define GPIO_ALT0		4
#define GPIO_ALT5		2

#define GPIO_FS(n)		\
	*(volatile unsigned int *)(GPIO_BASE + (((n) / 10) * 4))
#define SET_GPIO_FS(n, v) ({	\
	GPIO_FS(n) &= ~(7 << ((n) % 10 * 3)); \
	GPIO_FS(n) |= (v) << ((n) % 10 * 3); \
})

#define GPIO_SET(n) ({	\
	if (n / 32)		\
		*(volatile unsigned int *)(GPIO_BASE + 0x20) = 1 << ((n) % 32); \
	else 			\
		*(volatile unsigned int *)(GPIO_BASE + 0x1c) = 1 << (n); \
})
#define GPIO_CLEAR(n) ({	\
	if (n / 32)		\
		*(volatile unsigned int *)(GPIO_BASE + 0x2c) = 1 << ((n) % 32); \
	else			\
		*(volatile unsigned int *)(GPIO_BASE + 0x28) = 1 << (n); \
})

void gpio_pull(unsigned int pin, unsigned int mode);

static inline void ret_from_gpio_int(unsigned int n)
{
	struct gpio *gpio = (struct gpio *)GPIO_BASE;

	if (n / 32)
		gpio->event1 = 1 << (n % 32);
	else
		gpio->event0 = 1 << n;
}

#endif /* __RPI_GPIO_H__ */
