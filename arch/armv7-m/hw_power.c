#include "kernel/power.h"
#include "arch/hw_sysclk.h"
#include "arch/regs.h"
#include "io.h"

enum system_control_bits {
	SLEEPONEXIT	= 1U,
	SLEEPDEEP	= 2U,
	SEVONPEND	= 4U,
};

void hw_enter_sleep_nap(void)
{
	SCB_SCR &= ~(1UL << SLEEPDEEP);
	/*  drain any pending memory activity before suspending execution */
	dsb();
	__asm__ __volatile__("wfi" ::: "memory");
}

void hw_sleep_on_exit(void)
{
	SCB_SCR |= 1UL << SLEEPONEXIT;
}
