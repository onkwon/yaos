/* TODO: return back in the context as it was */
#include <foundation.h>
#include <kernel/task.h>
#include <asm/pinmap.h>

static const char *rname[] = { "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
		"r0", "r1", "r2", "r3", "r12", "lr", "pc", "psr" };

void print_context(unsigned int *regs)
{
	unsigned int i;

	for (i = 0; i < NR_CONTEXT; i++)
		printk("  %s\t 0x%08x <%08x>\n", rname[i], &regs[i], regs[i]);
}

void print_kernel_status(unsigned int *sp, unsigned int lr, unsigned int psr)
{
	printk("  kernel  SP	%08x\n", sp);
	printk("  stacked PSR	%08x\n", sp[7]);
	printk("  stacked PC	%08x\n", sp[6]);
	printk("  stacked LR	%08x\n", sp[5]);
	printk("  current LR	%08x\n", lr);
	printk("  current PSR	%08x(vector number:%d)\n", psr, psr & 0x1ff);
}

void print_user_status(unsigned int *sp)
{
	printk( "  user SP	%08x\n"
		"  stacked PSR	%08x\n"
		"  stacked PC	%08x\n"
		"  stacked LR	%08x\n",
		sp, sp[7], sp[6], sp[5]);
}

void print_task_status(struct task *task)
{
	printk("  task->sp	%08x\n", task->mm.sp);
	printk("  task->base	%08x\n", task->mm.base);
	printk("  task->heap	%08x\n", task->mm.heap);
	printk("  task->kernel	%08x\n", task->mm.kernel.base);
	printk("  task->ksp	%08x\n", task->mm.kernel.sp);
	printk("  task->state	%08x\n", task->state);
	printk("  task->irqflag %08x\n", task->irqflag);
	printk("  task->addr	%08x\n", task->addr);
	printk("  task		%08x\n", task);
	printk("  parent	%08x\n", task->parent);
	printk("  parent->addr	%08x\n", task->parent->addr);
}

static void __attribute__((naked)) isr_break()
{
	dsb();
	__context_save(current);

	unsigned int sp, lr, psr, usp;

	sp  = __get_sp();
	psr = __get_psr();
	lr  = __get_lr();
	usp = __get_usp();

	printk("\nKernel Space\n");
	print_kernel_status((unsigned int *)sp, lr, psr);

#if 0
	/* FIXME: unaligned access fault */
	printk("sp %x usp %x\n", sp, usp);
	printk("\nUser Space\n");
	print_user_status((unsigned int *)usp);
#endif

	printk("\nTask Status\n");
	print_task_status(current);

	printk("\nCurrent Context\n");
	print_context((unsigned int *)current->mm.sp);

	__context_restore(current);
	dsb();
	isb();

	ret_from_gpio_int(PIN_DEBUG);
	__ret();
}

static void inspect()
{
	int nvector;

	nvic_set_pri(23, 0); /* make it the most priority */

	nvector = gpio_init(PIN_DEBUG,
			GPIO_MODE_INPUT | GPIO_CONF_PULLUP | GPIO_INT_FALLING);

	register_isr(nvector, isr_break);
}
REGISTER_TASK(inspect, TASK_PRIVILEGED, HIGHEST_PRIORITY, STACK_SIZE_MIN);
