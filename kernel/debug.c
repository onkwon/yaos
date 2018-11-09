/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

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

void swo_putc(const int v)
{
	if ((ITM->TCR & 1UL) == 0UL)
		return;
	//if ((ITM->TER & (1UL << ch)) == 0)
	if ((ITM->TER & 1UL) == 0UL)
		return;

	while (ITM->PORT[0].u32 == 0UL) ;
	//if (ITM->PORT[0].u32 == 0)
	//	return 0;

	//ITM->PORT[0].u16 = 0x08 | (c << 8);
	ITM->PORT[0].u8 = (uint8_t)v;

	//return 1;
}

void swo_init(unsigned int ch, unsigned int baudrate)
{
	unsigned int prescaler = (get_hclk() / baudrate) - 1;

	COREDEBUG->DEMCR = 1UL << 24; // enable DWT and ITM

	*((volatile unsigned *)0xE0042004) = 0x00000027;

	TPIU_SPPR = 0x00000002; // 2: SWO NRZ, 1: SWO Manchester encoding
	TPIU_ACPR = prescaler;

	 /* ITM Lock Access Register, C5ACCE55 enables more write access to Control Register 0xE00 :: 0xFFC */
	*((volatile unsigned *)(ITM_BASE + 0x00FB0)) = 0xC5ACCE55;

	ITM->TCR = (0x7fUL << 16) | (1UL << 4) | (1UL << 2) | (1UL << 0);
	ITM->TPR = 0xf;
	ITM->TER = ch;

	DWT_CTRL = 0x400003FE;
	TPIU_FFCR = 0x00000100;
}

#include <kernel/init.h>

void __init debug_init()
{
	swo_init(1, 2000000);

#ifdef PIN_DEBUG
	int lvector;

	//nvic_pri_set(23, 0); /* make it the highest priority */

	lvector = gpio_init(PIN_DEBUG,
			GPIO_MODE_INPUT | GPIO_CONF_PULLDOWN | GPIO_INT_RISING);

	register_isr(lvector, ISR_break);
#endif
}
