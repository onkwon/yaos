#include "kernel/power.h"
#include "kernel/interrupt.h"
#include "kernel/systick.h"
#include "arch/regs.h"
#include "arch/mach/regs.h"
#include "arch/mach/hw_clock.h"
#include "io.h"

enum system_control_bits {
	SLEEPONEXIT	= 1U,
	SLEEPDEEP	= 2U,
	SEVONPEND	= 4U,
};

enum power_control_bits {
	LPDS		= 0,
	PDDS		= 1,
	CWUF		= 2,
	FPDS		= 9,
	LPUDS		= 10,
	MRUDS		= 11,
	VOS		= 14,
	ODEN		= 16,
	ODSWEN		= 17,
	UDEN		= 18,
};

/* all clocks in the 1.8 V domain are stopped.
 * The PLL, HSI and HSE RC oscillators are disabled.
 * SRAM and register contents are preserved.
 * all I/O pins keep the same state as in the Run mode.
 * ADC, DAC, WDG, RTC, LSI_RC, and LSE_OSC can consume power. */
void hw_enter_sleep_deep(void)
{
	SCB_SCR |= 1 << SLEEPDEEP;
	PWR_CR &= ~(1 << PDDS);

	systick_stop();

	/*  drain any pending memory activity before suspending execution */
	dsb();
	__asm__ __volatile__("wfi" ::: "memory");

	clock_init();
	systick_start();

	/* wakeup latency:
	 * HSI RC wakeup time + regulator wakeup time from Low-power mode */

	/* Note from reference manual:
	 * If the application needs to disable the external clock before
	 * entering Stop mode, the HSEON bit must first be disabled and the
	 * system clock switched to HSI. Otherwise, if the HSEON bit remains
	 * enabled and the external clock (external oscillator) is removed when
	 * entering Stop mode, the clock security system (CSS) feature must be
	 * enabled to detect any external oscillator failure and avoid a
	 * malfunction behavior when entering stop mode. */
	//BITBAND(&RCC_CR, CSSON, ON);
}

void hw_enter_sleep_blackout(void)
{
}
