#ifndef __YAOS_STM32_EXTI_H__
#define __YAOS_STM32_EXTI_H__

#include "arch/io.h"
#include "arch/mach/gpio.h"
#include "arch/mach/regs.h"
#include <stdbool.h>

static inline unsigned int get_exti_pending(void)
{
	return EXTI_PR;
}

static inline void clear_exti_pending(const int pin)
{
	BITBAND(&EXTI_PR, pin2portpin(pin), true);
}

void exti_enable(int pin, const bool enable);

#endif /* __YAOS_STM32_EXTI_H__ */
