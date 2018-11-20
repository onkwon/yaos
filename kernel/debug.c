#include "kernel/debug.h"
#include "arch/regs.h"
#include "arch/mach/clock.h"

void debug_init(void)
{
	hw_debug_init(1, 2000000);
}

///////////////////////////////////////////////////////////////////////////
// TEST
///////////////////////////////////////////////////////////////////////////

#include "kernel/init.h"
#include <stdio.h>

extern uintptr_t _init_func_list;

static void __init sys_init(void)
{
	uintptr_t *p = (uintptr_t *)&_init_func_list;

	while (*p)
		((void (*)(void))*p++)();
}

void kernel_init(void)
{
	sys_init();
	debug_init(); /* must follow sys_init() because of clock frequency dependency */

	while (1) {
		printf("Hello, World!\r\n");
	};
}
