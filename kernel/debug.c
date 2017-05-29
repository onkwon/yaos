#include <foundation.h>
#include <kernel/task.h>
#include <asm/pinmap.h>

static const char *rname[] = { "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
		"exc", "r0", "r1", "r2", "r3", "r12", "lr", "pc", "psr" };

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
	printk("  task		%08x %s\n", task, task->name);
	printk("  parent	%08x %s\n", task->parent, task->parent->name);
	printk("  parent->addr	%08x\n", task->parent->addr);
}

#ifdef PIN_DEBUG
/* externally triggered break point */
static void ISR_break()
{
	unsigned int sp, lr, psr, usp;

	sp  = __get_sp();
	psr = __get_psr();
	lr  = __get_lr();
	usp = __get_usp();

	printk("\nKernel Space\n");
	print_kernel_status((unsigned int *)sp, lr, psr);

	printk("\nUser Space\n");
	print_user_status((unsigned int *)usp);

	printk("\nTask Status\n");
	print_task_status(current);

	printk("\nCurrent Context\n");
	print_context((unsigned int *)current->mm.sp);
}
#endif

#include <kernel/init.h>

void __init debug_init()
{
#ifdef PIN_DEBUG
	int lvector;

	//nvic_set_pri(23, 0); /* make it the highest priority */

	lvector = gpio_init(PIN_DEBUG,
			GPIO_MODE_INPUT | GPIO_CONF_PULLDOWN | GPIO_INT_RISING);

	register_isr(lvector, ISR_break);
#endif
}
