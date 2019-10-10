#include "arch/hw_sysclk.h"
#include "arch/regs.h"
#include "arch/mach/hw_clock.h"

void hw_sysclk_reset(void)
{
	STK_VAL = 0;
}

void hw_sysclk_period_set(unsigned long period)
{
	STK_LOAD = period - 1;
}

unsigned long hw_sysclk_period_get(void)
{
	return STK_LOAD + 1;
}

void hw_sysclk_run(void)
{
#define SYSTICK_INT	2U
	STK_CTRL = (STK_CTRL & ~3UL) | (SYSTICK_INT) | 1UL;
}

void hw_sysclk_stop(void)
{
	STK_CTRL &= ~3UL;
}

unsigned long hw_sysclk_freq_get(void)
{
	return hw_clock_get_stk();
}
