#include <io.h>
#include <kernel/task.h>

int register_isr(unsigned int nvector, void (*func)())
{
	extern unsigned int _ram_start;
	*((unsigned int *)&_ram_start + nvector) = (unsigned int)func;
	dsb();

	return 0;
}

#include <kernel/lock.h>

static DEFINE_SPINLOCK(nvic_lock);

void nvic_set(unsigned int nirq, int on)
{
	reg_t *reg;
	unsigned int bit, base;

	bit  = nirq % 32;
	nirq = nirq / 32 * 4;
	base = on? NVIC_BASE : NVIC_BASE + 0x80;
	reg  = (reg_t *)(base + nirq);

	spin_lock(&nvic_lock);
	*reg = 1 << bit;
	spin_unlock(&nvic_lock);

	dsb();
}

void nvic_set_pri(unsigned int nirq, unsigned int pri)
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

#ifdef CONFIG_SYSCALL
#include <kernel/syscall.h>

#ifdef CONFIG_DEBUG_SYSCALL
unsigned int syscall_count = 0;
#endif

void __attribute__((naked)) svc_handler()
{
#ifdef CONFIG_DEBUG_SYSCALL
	/* FIXME: Do not use the code below as it will corrupt the context. */
	__asm__ __volatile__("add %0, %1, #1"
			: "=r"(syscall_count)
			: "0"(syscall_count)
			: "r0", "memory");
#endif
	__asm__ __volatile__(
			"push	{r9, lr}		\n\t"
			"mrs	r9, psp			\n\t"
			/* r0 must be the same value to the stacked one
			 * because of hardware mechanism. The meantime of
			 * entering into interrupt service routine
			 * nothing can change the value. But the problem
			 * is that it sometimes gets corrupted. how? why?
			 * So here I get r0 from stack. */
			"ldr	r0, [r9]		\n\t"
			/* if nr >= SYSCALL_NR */
			"cmp	r0, %0			\n\t"
			"it	ge			\n\t"
			/* then nr = 0 */
			"movge	r0, #0			\n\t"
			/* get the syscall address */
			"ldr	r3, =syscall_table	\n\t"
			"ldr	r3, [r3, r0, lsl #2]	\n\t"
			/* arguments in place */
			"ldr	r2, [r9, #12]		\n\t"
			"ldr	r1, [r9, #8]		\n\t"
			"ldr	r0, [r9, #4]		\n\t"
			"blx	r3			\n\t"
			/* store return value */
			"str	r0, [r9]		\n\t"
			"pop	{r9, pc}		\n\t"
			:: "I"(SYSCALL_NR)
			: "memory");
}
#endif /* CONFIG_SYSCALL */
