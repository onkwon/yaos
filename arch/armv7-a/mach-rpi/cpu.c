#include <foundation.h>
#include <kernel/init.h>

#define NR_VECTOR	8

static void __init mem_init()
{
	unsigned int i;

	/* copy interrupt vector table to 0x0 */
	for (i = 0; i < NR_VECTOR * WORD_SIZE * 2; i += WORD_SIZE)
		*(unsigned int *)(0 + i) = *(unsigned int *)(0x8000 + i);

	/* clear .bss section */
	extern char _bss, _ebss;
	for (i = 0; (&_bss + i) < &_ebss; i++)
		*((char *)&_bss + i) = 0;

	dmb();
}
REGISTER_INIT(mem_init, 1);

#include <kernel/module.h>
#include <kernel/page.h>

static void alloc_irq_stack()
{
	unsigned int *sp = kmalloc(PAGESIZE);

	__asm__ __volatile__(
			"mrs	r1, cpsr		\n\t"
			"mov	r0, #0xd2		\n\t"
			"msr	cpsr_c, r0		\n\t"
			"mov	sp, %0			\n\t"
			:: "r"(sp) : "r0", "r1", "memory");
	__asm__ __volatile__(
			"msr	cpsr_c, r1		\n\t"
			::: "memory");
}
//MODULE_INIT(alloc_irq_stack);
/* it shares with kernel stack. so 2 stacks, irq == svc and user stack. */
