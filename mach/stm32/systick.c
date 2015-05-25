#include <foundation.h>
#include <kernel/sched.h>
#include <kernel/jiffies.h>
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
	SET_SYSTIMER(get_systick_hz() / HZ - 1);
}

unsigned int get_systick_hz()
{
	return get_stkclk(get_hclk(get_sysclk()));
}
