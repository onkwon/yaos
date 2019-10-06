#include "io.h"
#include "syslog.h"
#include "heap.h"

#include "kernel/init.h"
#include "kernel/interrupt.h"
#include "kernel/debug.h"
#include "kernel/task.h"
#include "kernel/systick.h"
#include "kernel/sched.h"
#include "kernel/timer.h"
#include "arch/mach/hw_clock.h"

#include <stdio.h>
#include <stdint.h>

extern const uintptr_t _etext, _edata, _ebss;
extern uintptr_t _data, _bss;
extern uintptr_t _init_func_list, _driver_list;
#if !defined(CONFIG_SCHEDULER)
extern int main(void);
#endif

static inline void mem_init(void)
{
	const uintptr_t *end, *src;
	uintptr_t *dst;
	uintptr_t i;

	/* copy .data section from flash to sram */
	dst = (uintptr_t *)&_data;
	end = (const uintptr_t *)&_edata;
	src = (const uintptr_t *)&_etext;

	for (i = 0; &dst[i] < end; i++)
		dst[i] = src[i];

	/* clear .bss section */
	dst = (uintptr_t *)&_bss;
	end = (const uintptr_t *)&_ebss;

	for (i = 0; &dst[i] < end; i++)
		dst[i] = 0;

	dsb();
}

static void __init early_drv_init(void)
{
	uintptr_t *func = (uintptr_t *)&_init_func_list;

	while (*func)
		((void (*)(void))*func++)();
}

static void __init drv_init(void)
{
	uintptr_t *func = (uintptr_t *)&_driver_list;

	while (*func)
		((void (*)(void))*func++)();
}

void freeze(void)
{
	debug("freeze");
	while (1);
}

void __init kernel_init(void)
{
	mem_init();
	irq_init();
	early_drv_init();

	debug_init(); /* it must be called after drv_init() because of clock
			 frequency dependency */
	info("yaos %s %s running at %lu MHz, %s",
			def2str(VERSION), def2str(MACHINE),
			hw_clock_get_hclk() / MHZ, __DATE__);

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
