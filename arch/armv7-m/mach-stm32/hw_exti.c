/** @file exti.c */
#include "arch/mach/hw_exti.h"
#include "arch/mach/hw_clock.h"
#include "arch/atomic.h"

void hw_exti_enable(uint16_t pin, const bool enable)
{
	reg_t *reg;
	unsigned int port, bit, val;

	port = gpio_to_port(pin);
	pin = gpio_to_ppin(pin);
	bit = (unsigned int)(pin % 4 * 4);
	pin = (uint16_t)(pin / 4 * 4);
	reg = (reg_t *)((SYSCFG_BASE + 8) + pin);

	do {
		val = __ldrex(reg);
		val &= ~(0xfUL << bit);
		val |= (port * enable) << bit;
	} while (__strex(val, reg));

	if (!(hw_clock_get_apb2() & (1UL << RCC_SYSCFGEN_BIT)))
		hw_clock_set_apb2(RCC_SYSCFGEN_BIT, true);
}
