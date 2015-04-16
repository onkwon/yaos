#include <foundation.h>
#include <clock.h>
#include <time.h>
#include <sched.h>

static void __attribute__((naked)) isr_systick()
{
	/* All interrupts get disabled and current context is saved by
	 * schedule_prepare(). Contrary it gets enabled and restored by
	 * schedule_finish(). */
	schedule_prepare();

	SYSTICK_FLAG(); /* clear flag */
	ticks_64 += STK_LOAD + 1;

	schedule_core();
	schedule_finish();

	__asm__ __volatile__("bx lr");
}

#include <asm/clock.h>

void systick_init()
{
	ISR_REGISTER(15, isr_systick);

	RESET_SYSTICK();
	SET_SYSTICK(get_stkclk(get_hclk(get_sysclk())) / HZ - 1);
	SYSTICK(ON | SYSTICK_INT);
}
