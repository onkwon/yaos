#include <io.h>
#include <kernel/task.h>
#include <error.h>

void isr_null(int nvec)
{
	error("ISR is not yet registered: %x", nvec);
}

#ifdef CONFIG_IRQ_HIERARCHY
void (*vector_table[NVECTOR_MAX])(int nvec);

int register_irq(int nvector, void (*handler)(int nvec))
{
	if (nvector < NVECTOR_IRQ || nvector >= NVECTOR_MAX)
		return ERANGE;

	vector_table[nvector] = handler;
	dsb();
	isb();

	return 0;
}

int register_isr(int nvector, void (*handler)(int nvec))
{
	return register_irq(nvector, handler);
}

void __attribute__((naked)) irq_handler()
{
	__asm__ __volatile__(
			"sub	sp, sp, #8		\n\t"
			"str	lr, [sp]		\n\t"
			"mrs	r0, ipsr		\n\t"
			"ldr	r1, =vector_table	\n\t"
			"mov	r2, r0, lsl #2		\n\t"
			"ldr	r3, [r1, r2]		\n\t"
			"blx	r3			\n\t"
			"ldr	lr, [sp]		\n\t"
			"add	sp, sp, #8		\n\t"
			"bx	lr			\n\t"
			::: "memory");
}
#else
int register_isr(int nvector, void (*handler)(int nvec))
{
	extern unsigned int _ram_start;
	*((unsigned int *)&_ram_start + nvector) = (unsigned int)handler;
	dsb();
	isb();

	return 0;
}

int register_irq(int nvector, void (*handler)(int nvec))
{
	return 0;
}

void irq_handler()
{
	isr_null(__get_psr());
}
#endif /* CONFIG_IRQ_HIERARCHY */

#ifdef CONFIG_SYSCALL
#include <kernel/syscall.h>

#ifdef CONFIG_DEBUG_SYSCALL
unsigned int syscall_count = 0;
#endif

#ifndef CONFIG_SYSCALL_THREAD
#ifdef CONFIG_DEBUG_SYSCALL_NESTED
#include <error.h>
void syscall_nested(int sysnum)
{
	error("syscall %d nested!! current %x %s",
			sysnum, current, current->name);
}
#endif
#endif

void __attribute__((naked)) svc_handler()
{
	__asm__ __volatile__(
#ifdef CONFIG_DEBUG_SYSCALL
			"ldr	r0, =syscall_count	\n\t"
			"ldr	r1, [r0]		\n\t"
			"add	r1, #1			\n\t"
			"str	r1, [r0]		\n\t"
#endif
			"mrs	r12, psp		\n\t"
			/* get the sysnum requested */
			"ldr	r0, [r12]		\n\t"
			"teq	r0, %0			\n\t"
			"beq	sys_schedule		\n\t"
			/* #24 = r0-r3, lr and padding(4) */
			"sub	sp, sp, #24		\n\t"
			"str	lr, [sp, #20]		\n\t"
#ifndef CONFIG_SYSCALL_THREAD
#ifdef CONFIG_DEBUG_SYSCALL_NESTED
			"stmia	sp, {r0-r3}		\n\t"
			"ldr	r1, =current		\n\t"
			"ldr	r2, [r1]		\n\t"
			"ldr	r1, [r2, #4]		\n\t"
			"tst	r1, %2			\n\t"
			"it	ne			\n\t"
			"blne	syscall_nested		\n\t"
			"ldmia	sp, {r0-r3}		\n\t"
#endif
#endif
			/* save context that are not yet saved by hardware.
			 * you can remove this overhead if not using
			 * `syscall_delegate_atomic()` but using only
			 * CONFING_SYSCALL_THREAD. */
#ifndef CONFIG_SYSCALL_THREAD
			"sub	r1, r12, #32		\n\t"
			"msr	psp, r1			\n\t"
			"stmdb	r12, {r4-r11}		\n\t"
#endif
			/* if nr >= SYSCALL_NR */
			"cmp	r0, %1			\n\t"
			"it	ge			\n\t"
			/* then nr = 0 */
			"movge	r0, #0			\n\t"
			/* get handler address */
			"ldr	r3, =syscall_table	\n\t"
			"ldr	r3, [r3, r0, lsl #2]	\n\t"
			/* arguments in place */
			"ldr	r0, [r12, #4]		\n\t"
			"ldr	r1, [r12, #8]		\n\t"
			"ldr	r2, [r12, #12]		\n\t"
			"blx	r3			\n\t"
			"mrs	r12, psp		\n\t"
#ifndef CONFIG_SYSCALL_THREAD
			/* check if delegated task */
			"ldr	r1, =current		\n\t"
			"ldr	r2, [r1]		\n\t" /* read flags */
			"ldr	r1, [r2, #4]		\n\t"
			"ands	r1, %2			\n\t"
			/* restore saved context if not */
			"itt	eq			\n\t"
			"ldmiaeq	r12!, {r4-r11}		\n\t"
			"msreq	psp, r12		\n\t"
#endif
			/* store return value */
			"str	r0, [r12]		\n\t"
			"dsb				\n\t"
			"isb				\n\t"
			"ldr	lr, [sp, #20]		\n\t"
			"add	sp, sp, #24		\n\t"
			"bx	lr			\n\t"
			:: "I"(SYSCALL_SCHEDULE), "I"(SYSCALL_NR), "I"(TF_SYSCALL)
			: "r12", "memory");
}
#endif /* CONFIG_SYSCALL */

void nvic_set(int nirq, int on)
{
	reg_t *reg;
	unsigned int bit, base;

	bit  = nirq % 32;
	nirq = nirq / 32 * 4;
	base = on? NVIC_BASE : NVIC_BASE + 0x80;
	reg  = (reg_t *)(base + nirq);

	*reg = 1 << bit;

	dsb();
}

void nvic_set_pri(int nirq, int pri)
{
	reg_t *reg;
	unsigned int bit;

	bit  = nirq % 4 * 8;
	reg  = (reg_t *)((NVIC_BASE + 0x300) + (nirq / 4 * 4));
	*reg &= ~(0xff << bit);
	*reg |= ((pri & 0xf) << 4) << bit;

	dsb();
}

void __attribute__((naked)) sys_schedule()
{
	SCB_ICSR |= 1 << 28; /* raising pendsv for scheduling */
	__ret();
}

#include <kernel/init.h>

#ifdef CONFIG_IRQ_HIERARCHY
void __init irq_init()
{
	int i;

	for (i = 0; i < NVECTOR_MAX; i++)
		vector_table[i] = isr_null;
}
#endif
