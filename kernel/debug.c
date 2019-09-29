#include "kernel/debug.h"
#include "kernel/sched.h"
#include "drivers/gpio.h"
#include "arch/mach/board/pinmap.h"
#include "syslog.h"

void debug_print_context(uintptr_t *regs)
{
	static const char *rname[] = { "r4", "r5", "r6", "r7", "r8", "r9",
		"r10", "r11", "exc", "r0", "r1", "r2", "r3", "r12", "lr", "pc",
		"psr" };

	unsigned int i;

	for (i = 0; i < NR_CONTEXT; i++)
		printk("  %s\t 0x%08x <%08x>\n",
				rname[i], (uintptr_t)&regs[i], regs[i]);
}

void debug_print_kernel_status(uintptr_t *sp, uintptr_t lr, uintptr_t psr)
{
	printk("  kernel  SP	%08x\n", (uintptr_t)sp);
	printk("  stacked PSR	%08x\n", sp[7]);
	printk("  stacked PC	%08x\n", sp[6]);
	printk("  stacked LR	%08x\n", sp[5]);
	printk("  current LR	%08x\n", lr);
	printk("  current PSR	%08x(vector number:%u)\n", psr, psr & 0x1ff);
}

void debug_print_user_status(uintptr_t *sp)
{
	printk( "  user SP	%08x\n"
		"  stacked PSR	%08x\n"
		"  stacked PC	%08x\n"
		"  stacked LR	%08x\n",
		(uintptr_t)sp, sp[7], sp[6], sp[5]);
}

void debug_print_task_status(void *arg)
{
	struct task *task = arg;

	printk("  task->sp.base	%08x\n", (uintptr_t)task->stack.base);
	printk("  task->sp	%08x\n", (uintptr_t)task->stack.p);
	printk("  task->heap	%08x - %08x\n",
			(uintptr_t)task->heap.base, (uintptr_t)task->heap.limit);
	printk("  task->kernel	%08x\n", (uintptr_t)task->kstack.base);
	printk("  task->ksp	%08x\n", (uintptr_t)task->kstack.p);
	printk("  task->state	%08lx\n", task->state);
	printk("  task->irqflag %08x\n", task->irqflag);
	printk("  task->addr	%08x\n", (uintptr_t)task->addr);
	printk("  task		%08x %s\n", (uintptr_t)task, task->name);
	printk("  parent	%08x %s\n", (uintptr_t)task->parent, task->parent->name);
	printk("  parent->addr	%08x\n", (uintptr_t)task->parent->addr);
}

#ifdef PIN_DEBUG
/* externally triggered break point */
static void ISR_break(const int vector)
{
	uintptr_t sp, lr, psr, usp;

	sp  = __get_sp();
	psr = __get_psr();
	lr  = __get_lr();
	usp = __get_usp();

	debug("%d", vector);

	printk("\nKernel Space\n");
	debug_print_kernel_status((uintptr_t *)sp, lr, psr);

	printk("\nUser Space\n");
	debug_print_user_status((uintptr_t *)usp);

	printk("\nTask Status\n");
	debug_print_task_status(current);

	printk("\nCurrent Context\n");
	debug_print_context((uintptr_t *)current->stack.p);
}
#endif

void debug_putc(const int c)
{
	hw_debug_putc(c);
}

#include <kernel/init.h>

void __init debug_init(void)
{
	hw_debug_init(1, 2000000);

#ifdef PIN_DEBUG
	//nvic_pri_set(23, 0); /* make it the highest priority */

	gpio_init(PIN_DEBUG,
			GPIO_MODE_INPUT | GPIO_CONF_PULLUP | GPIO_INT_FALLING,
			ISR_break);
#endif
}
