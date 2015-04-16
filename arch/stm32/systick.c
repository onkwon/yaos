#include <foundation.h>
#include <sched.h>
#include <clock.h>
#include <time.h>

static void isr_systick()
{
	update_curr(STK_LOAD + 1);

	SCB_ICSR |= 1 << 28; /* raising pendsv for scheduling */
}

void systick_init()
{
	ISR_REGISTER(15, isr_systick);

	RESET_SYSTICK();
	SET_SYSTICK(get_stkclk(get_hclk(get_sysclk())) / HZ - 1);
}
