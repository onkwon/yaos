#include "io.h"
#include "syslog.h"
#include "heap.h"

#include "kernel/init.h"
#include "kernel/interrupt.h"
#include "kernel/task.h"
#include "kernel/systick.h"
#include "kernel/sched.h"
#include "kernel/timer.h"
#include "kernel/debug.h"

#include "arch/mach/hw_clock.h"

#include <stdio.h>
#include <stdint.h>

extern uintptr_t _driver_list;
#if !defined(CONFIG_SCHEDULER)
extern int main(void);
#endif

static void __init drv_init(void)
{
	uintptr_t *func = (uintptr_t *)&_driver_list;

	while (*func)
		((void (*)(void))*func++)();
}

void __init __attribute__((noreturn)) kernel_init(void)
{
	debug_init(); /* it must be called after early_drv_init() because of
			 clock frequency dependency */
	info("yaos %s %s running at %lu MHz, %s",
			def2str(VERSION), def2str(MACHINE),
			hw_clock_get_hclk() / MHZ, __DATE__);

	irq_init();

	heap_init();
	timer_init();

	systick_init(SYSTICK_HZ);
	systick_start();

	drv_init();

#if !defined(CONFIG_SCHEDULER)
	sei();

	main();
#else

	sched_init();
	task_init();

	set_user_sp(current->stack.p);
	set_kernel_sp(current->kstack.p);

	sei();

	resched();
#endif

	/* it doesn't really reach up to this point. init task becomes idle as
	 * its context is already set so */
	freeze();
}
