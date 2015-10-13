#include <clock.h>
#include <foundation.h>
#include "irq.h"

#define TIMER_FREQ		1000000	/* 1MHz */

unsigned int getclk()
{
	return 0;
}

unsigned int get_sysclk_freq()
{
	return TIMER_FREQ;
}

void run_sysclk()
{
	struct general_timer *gtimer = (struct general_timer *)GENTIMER_BASE;

	gtimer->control |= 1 << 7; /* timer enable */
}

void stop_sysclk()
{
	struct general_timer *gtimer = (struct general_timer *)GENTIMER_BASE;

	gtimer->control &= ~(1 << 7); /* timer enable */
}

unsigned int get_sysclk()
{
	struct general_timer *gtimer = (struct general_timer *)GENTIMER_BASE;

	return gtimer->value;
}

unsigned int get_sysclk_max()
{
	struct general_timer *gtimer = (struct general_timer *)GENTIMER_BASE;

	return gtimer->load + 1;
}

int sysclk_init()
{
	struct interrupt_controller *intc
		= (struct interrupt_controller *)INTCNTL_BASE;
	struct general_timer *gtimer
		= (struct general_timer *)GENTIMER_BASE;

	gtimer->load = get_sysclk_freq() / HZ - 1;
	gtimer->control =
		(1 << 1)	/* 32-bit counter */
		| (1 << 5);	/* interrupt enable */

	intc->basic_enable = 1; /* enable irq0, timer */

	return IRQ_TIMER;
}
