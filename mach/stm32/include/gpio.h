#ifndef __STM32_GPIO_H__
#define __STM32_GPIO_H__

#define NR_PORT			7
#define PINS_PER_PORT		16

#include <io.h>

static inline void ret_from_gpio_int(unsigned int n)
{
	unsigned int pin = n % PINS_PER_PORT;

	EXTI_PR |= 1 << pin;
}

#endif /* __STM32_GPIO_H__ */
