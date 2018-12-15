#include "kernel/init.h"
#include "kernel/interrupt.h"
#include "kernel/debug.h"
#include "io.h"

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

void __init system_init(void)
{
	mem_init();
	irq_init();

	kernel_init();
}
