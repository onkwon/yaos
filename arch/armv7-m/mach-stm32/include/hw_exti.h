#ifndef __YAOS_STM32_HW_EXTI_H__
#define __YAOS_STM32_HW_EXTI_H__

#include "arch/hw_io.h"
#include "arch/mach/gpio.h"
#include "arch/mach/regs.h"
#include <stdbool.h>
#include <stdint.h>

static inline uintptr_t hw_exti_get_pending(void)
{
	return EXTI_PR;
}

static inline void hw_exti_clear_pending(const uint16_t pin)
{
	BITBAND(&EXTI_PR, gpio_to_ppin(pin), true);
}

void hw_exti_enable(uint16_t pin, const bool enable);

#endif /* __YAOS_STM32_HW_EXTI_H__ */
