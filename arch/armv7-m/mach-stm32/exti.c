/** @file exti.c */
#include "arch/mach/exti.h"
#include "arch/mach/clock.h"
#include "arch/atomic.h"

void exti_enable(int pin, const bool enable)
{
	reg_t *reg;
	unsigned int port, bit, val;

	port = pin2port(pin);
	pin = pin2portpin(pin);
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
