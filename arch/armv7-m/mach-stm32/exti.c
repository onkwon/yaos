#include <kernel/task.h>
#include <asm/lock.h>

void exti_enable(int pin, bool enable)
{
	reg_t *reg;
	unsigned int port, bit, val;

	port = pin2port(pin);
	pin = pin2portpin(pin);
	bit = pin % 4 * 4;
	pin = pin / 4 * 4;
	reg = (reg_t *)((SYSCFG_BASE+8) + pin);

	do {
		val = __ldrex(reg);
		val &= ~(0xf << bit);
		val |= port << bit;
	} while (__strex(val, reg));

	if (!(__read_apb2_clock() & (1 << RCC_SYSCFGEN_BIT)))
		__turn_apb2_clock(RCC_SYSCFGEN_BIT, ON);

	(void)enable;
}
