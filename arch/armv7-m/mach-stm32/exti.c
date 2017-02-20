#include <io.h>
#include <kernel/task.h>

void link_exti_to_nvic(unsigned int port, unsigned int pin)
{
	reg_t *reg;
	unsigned int bit;

	bit = pin % 4 * 4;
	pin = pin / 4 * 4;
	reg = (reg_t *)((SYSCFG_BASE+8) + pin);
	*reg = MASK_RESET(*reg, 0xf << bit) | (port << bit);

	if (!(__read_apb2_clock() & (1 << RCC_SYSCFGEN_BIT)))
		__turn_apb2_clock(RCC_SYSCFGEN_BIT, ON);
}
