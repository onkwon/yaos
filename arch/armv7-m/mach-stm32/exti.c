#include <io.h>
#include <kernel/task.h>

void link_exti_to_nvic(unsigned int port, unsigned int pin)
{
	volatile unsigned int *reg;
	unsigned int bit;

	bit = pin % 4 * 4;
	pin = pin / 4 * 4;
	reg = (volatile unsigned int *)((AFIO_BASE+8) + pin);
	*reg = MASK_RESET(*reg, 0xf << bit) | port << bit;
}
