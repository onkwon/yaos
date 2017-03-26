#include <foundation.h>
#include <kernel/task.h>
#include <kernel/debug.h>

enum fault_type {
	USAGE_FAULT	= (1 << 3),
	BUS_FAULT	= (1 << 1),
	MM_FAULT	= (1 << 0),
};

enum usage_fault_status {
	UNDEFINSTR	= (1 << 16), /* Undefined instruction */
	INVSTATE	= (1 << 17), /* Invalid state of EPSR */
	INVPC		= (1 << 18), /* Invalid PC load by EXC_RETURN */
	NOCP		= (1 << 19), /* No coprocessor */
	UNALIGNED	= (1 << 24), /* Unaligned access */
	DIVBYZERO	= (1 << 25), /* Divide by zero */
};

enum bus_fault_status {
	IBUSERR		= (1 <<  8), /* Instruction bus error */
	PRECISERR	= (1 <<  9), /* Precise data bus error */
	IMPRECISERR	= (1 << 10), /* Imprecise data bus error */
	UNSTKERR	= (1 << 11), /* Fault on unstacking for a return from exception */
	STKERR		= (1 << 12), /* Fault on stacking for exception */
	BFARVALID	= (1 << 15), /* Address Register valid flag */
};

enum mm_fault_status {
	IACCVIOL	= (1 << 0), /* Instruction access violation flag */
	DACCVIOL	= (1 << 1), /* Data access violation flag */
	MUNSTKERR	= (1 << 3), /* Fault on unstacking for a return from exception */
	MSTKERR		= (1 << 4), /* Fault on stacking for exception */
	MMARVALID	= (1 << 7), /* Address register valid flag */
};

static inline void busfault()
{
	if (SCB_CFSR & IBUSERR)
		printk("Instruction bus error");
	if (SCB_CFSR & PRECISERR)
		printk("Precise data bus error");
	if (SCB_CFSR & IMPRECISERR)
		printk("Imprecise data bus error");
	if (SCB_CFSR & UNSTKERR)
		printk("on unstacking for a return from exception");
	if (SCB_CFSR & STKERR)
		printk("on stacking for exception");

	if (SCB_CFSR & BFARVALID)
		printk("\nFault address: 0x%08x", SCB_BFAR);
}

static inline void usagefault()
{
	if (SCB_CFSR & UNDEFINSTR)
		printk("Undefined instruction");
	if (SCB_CFSR & INVSTATE)
		printk("Invalid state of EPSR");
	if (SCB_CFSR & INVPC)
		printk("Invalid PC load by EXC_RETURN");
	if (SCB_CFSR & NOCP)
		printk("No coprocessor");
	if (SCB_CFSR & UNALIGNED)
		printk("Unalignd access");
	if (SCB_CFSR & DIVBYZERO)
		printk("Divide by zero");
}

#define IS_FROM_THREAD(lr)	!!((lr) & (1 << 3))

void __attribute__((naked)) isr_fault()
{
	dsb();
	__context_save(current);

	unsigned int sp, lr, psr, usp;

	sp  = __get_sp();
	psr = __get_psr();
	lr  = __get_lr();
	usp = __get_usp();

	error("\n\nFault type: %x <%08x>\n"
		"  (%x=Usage fault, %x=Bus fault, %x=Memory management fault)",
		SCB_SHCSR & 0xfff, SCB_SHCSR, USAGE_FAULT, BUS_FAULT, MM_FAULT);

	printk("Fault source: ");
	busfault();
	usagefault();

	printk("\nSCB_ICSR  0x%08x\n"
		"SCB_CFSR  0x%08x\n"
		"SCB_HFSR  0x%08x\n"
		"SCB_MMFAR 0x%08x\n"
		"SCB_BFAR  0x%08x\n",
		SCB_ICSR, SCB_CFSR, SCB_HFSR, SCB_MMFAR, SCB_BFAR);

	printk("\nKernel Space\n");
	print_kernel_status((unsigned int *)sp, lr, psr);

	printk("\nUser Space\n");
	print_user_status((unsigned int *)usp);

	printk("\nTask Status\n");
	print_task_status(current);

	printk("\nCurrent Context\n");
	print_context((unsigned int *)current->mm.sp);

	if (IS_FROM_THREAD(lr) && current != &init) {
		error("Kill current %s(%x)", current->name, current);

		sys_kill_core(current, current);
		schedule_core();
		__context_restore(current);
		dsb();
		isb();
		__ret();
	}

	freeze();
}
