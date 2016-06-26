#ifndef __STM32_GPIO_H__
#define __STM32_GPIO_H__

#define NR_PORT			7
#define PINS_PER_PORT		16

#include <io.h>
#include <kernel/lock.h>

static inline void ret_from_gpio_int(unsigned int n)
{
#ifdef CONFIG_SMP
	extern lock_t gpio_irq_lock;
#endif
	unsigned int irqflag;

	unsigned int pin = n % PINS_PER_PORT;

	spin_lock_irqsave(&gpio_irq_lock, irqflag);
	EXTI_PR |= 1 << pin;
	spin_unlock_irqrestore(&gpio_irq_lock, irqflag);
}

#endif /* __STM32_GPIO_H__ */
