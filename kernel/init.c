/*
 * init.c - kernel initializaion
 *
 * (C) 2015-2019 Kyunghwan Kwon <kwon@toanyone.net>
 *
 * This code is licensed under the Apache License, Version 2.0.
 */

#include "kernel/init.h"
#include "kernel/interrupt.h"
#include "kernel/debug.h"
#include "kernel/task.h"
#include "kernel/systick.h"
#include "kernel/sched.h"
#include "io.h"
#include "syslog.h"

#include <stdio.h>
#include <stdint.h>

extern const uintptr_t _etext, _edata, _ebss;
extern uintptr_t _data, _bss;
extern uintptr_t _init_func_list;

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

#include "kernel/init.h"

static void __init drv_init(void)
{
	uintptr_t *p = (uintptr_t *)&_init_func_list;

	while (*p)
		((void (*)(void))*p++)();
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
	drv_init();

	debug_init(); /* it must be called after drv_init() because of clock
			 frequency dependency */
	task_init();
	systick_init(1);
	set_user_sp(current->stack.p);
	set_kernel_sp(current->kstack.p);
	systick_start();

	sei();

	resched();

	/* it doesn't really reach up to this point. init task becomes idle as
	 * its context is already set so */
	freeze();
}
