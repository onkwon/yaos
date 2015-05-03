#include <foundation.h>
#include <kernel/sched.h>
#include <clock.h>

static void isr_systick()
{
	update_tick(1);
	update_curr();

	SCB_ICSR |= 1 << 28; /* raising pendsv for scheduling */
}

void systick_init()
{
	ISR_REGISTER(15, isr_systick);

	RESET_SYSTIMER();
	SET_SYSTIMER(get_stkclk(get_hclk(get_sysclk())) / HZ - 1);
}
