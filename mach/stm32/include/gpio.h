#ifndef __STM32_GPIO_H__
#define __STM32_GPIO_H__

#define NR_PORT			7
#define PINS_PER_PORT		16

#include <io.h>
#include <kernel/lock.h>

static inline void ret_from_gpio_int(unsigned int n)
{
	extern lock_t gpio_irq_lock;
	unsigned int pin = n % PINS_PER_PORT;

	spin_lock(gpio_irq_lock);
	EXTI_PR |= 1 << pin;
	spin_unlock(gpio_irq_lock);
}

#endif /* __STM32_GPIO_H__ */
