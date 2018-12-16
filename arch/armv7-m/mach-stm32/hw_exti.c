/** @file exti.c */
#include "arch/mach/hw_exti.h"
#include "arch/mach/clock.h"
#include "arch/atomic.h"

void hw_exti_enable(uint16_t pin, const bool enable)
{
	reg_t *reg;
	unsigned int port, bit, val;

	port = gpio_to_port(pin);
	pin = gpio_to_ppin(pin);
	bit = pin % 4 * 4;
	pin = pin / 4 * 4;
	reg = (reg_t *)((SYSCFG_BASE + 8) + pin);

	do {
		val = __ldrex(reg);
		val &= ~(0xfUL << bit);
		val |= (port * enable) << bit;
	} while (__strex(val, reg));

	if (!(__read_apb2_clock() & (1UL << RCC_SYSCFGEN_BIT)))
		__turn_apb2_clock(RCC_SYSCFGEN_BIT, true);
}
