#include "kernel/syscall.h"
#include "kernel/task.h"
#include "syslog.h"

#if defined(CONFIG_SYSCALL)
int syscall(int n, ...)
{
	svc(SYSCALL_NR);
	(void)n;

	register int result __asm__("r0");

	return result;
}

static int __attribute__((used)) syscall_nested(int sysnum)
{
	debug("syscall can not be nested: %d", sysnum);
	while (1);
	return 0;
}

/*
 * [21:84]             (sp before system call)
 * [20:80] PSR         -
 * [19:76] PC          |
 * [18:72] LR          |
 * [17:68] R12         |
 * [16:64] R3(a2)      |- saved by hardware(8)
 * [15:60] R2(a1)      |
 * [14:56] R1(a0)      |
 * [13:52] R0(sysnum)  -
 * [12:48] EXC_RETURN  -
 * [11:44] R11         |
 * [10:40] R10         |
 * [09:36] R9          |
 * [08:32] R8          |- saved by software(9)
 * [07:28] R7          |
 * [06:24] R6          |
 * [05:20] R5          |
 * [04:16] R4          -
 * [03:12] padding
 * [02:08] flags
 * [01:04] flags(addr)
 * [00:00] handler     (sp when exit from system call)
 *         PSR         -
 *         PC(callback)|
 *         LR          |
 *         R12         |
 *         R3          |- copy and manipulated context
 *         R2          |
 *         R1          |
 *         R0          -
 */

#if defined(CONFIG_DEBUG_SYSCALL)
unsigned long nr_syscalled;
#endif

void __attribute__((naked)) ISR_svc(void)
{
	__asm__ __volatile__(
#if defined(CONFIG_DEBUG_SYSCALL)
			"ldr	r0, =nr_syscalled	\n\t"
			"ldr	r1, [r0]		\n\t"
			"add	r1, #1			\n\t"
			"str	r1, [r0]		\n\t"
#endif /* CONFIG_DEBUG_SYSCALL */
			"mrs	r12, psp		\n\t"
			/* get the sysnum requested */
			"ldr	r0, [r12]		\n\t"
			/* save exc_return */
			"push	{lr}			\n\t"
#if defined(CONFIG_SYSCALL_DELEGATE)
#if defined(CONFIG_DEBUG_SYSCALL_NESTED)
			"push	{r0-r3}			\n\t"
			"ldr	r1, =current		\n\t"
			"ldr	r2, [r1]		\n\t"
			"ldr	r1, [r2, #4]		\n\t"
			"tst	r1, %1			\n\t"
			"it	ne			\n\t"
			"blne	syscall_nested		\n\t"
			"pop	{r0-r3}			\n\t"
#endif /* CONFIG_DEBUG_SYSCALL_NESTED */
#endif /* CONFIG_SYSCALL_DDELEGATE */
			/* save context that are not yet saved by hardware.
			 * you can remove this overhead if not using
			 * `syscall_delegate_atomic()` but using only
			 * CONFING_SYSCALL_THREAD. */
#if defined(CONFIG_SYSCALL_DELEGATE)
			"sub	r1, r12, #36		\n\t"
			"msr	psp, r1			\n\t"
			"stmdb	r12, {r4-r11, lr}	\n\t"
#endif /* CONFIG_SYSCALL_DELEGATE */
			/* if nr >= SYSCALL_NR */
			"cmp	r0, %0			\n\t"
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
			/* exc_return */
			"pop	{lr}			\n\t"
#if defined(CONFIG_SYSCALL_DELEGATE)
			/* check if delegated task */
			"ldr	r1, =current		\n\t"
			"ldr	r2, [r1]		\n\t" /* read flags */
			"ldr	r1, [r2, #4]		\n\t"
			"ands	r1, %1			\n\t"
			/* restore saved context if not */
			"itte	eq			\n\t"
			"ldmiaeq	r12!, {r4-r11, lr}		\n\t"
			"msreq	psp, r12		\n\t"
			/* manipulate exc_return */
			"orrne	lr, #0x10		\n\t"
#endif /* CONFIG_SYSCALL_DELEGATE */
			/* store return value */
			"str	r0, [r12]		\n\t"
			"dsb				\n\t"
			"isb				\n\t"
			"bx	lr			\n\t"
			:: "I"(SYSCALL_NR), "I"(TF_SYSCALL)
			: "r12", "memory");
}

void syscall_delegate_callback(void)
{
	__asm__ __volatile__(
			/* arguments in place */
			"ldr	r0, [sp, #56]			\n\t" /* r1 */
			"ldr	r1, [sp, #60]			\n\t" /* r2 */
			"ldr	r2, [sp, #64]			\n\t" /* r3 */
			"ldr	r3, [sp]			\n\t" /* handler */
			"blx	r3				\n\t"
			/* restore user context saved by hardware */
			"ldr	r12, [sp, #68]			\n\t"
			"ldr	lr, [sp, #72]			\n\t"
			"ldr	r1, [sp, #76]			\n\t" /* pc */
			/* set LSB of return address */
			"orr	r1, #1				\n\t"
			/* back to the original state */
			"ldr	r2, [sp, #4]			\n\t" /* flags(addr) */
			"ldr	r3, [sp, #8]			\n\t" /* flags */
			"str	r3, [r2]			\n\t"
			"ands	r3, %1				\n\t"
			"ite	eq				\n\t"
			"moveq	r2, #3				\n\t"
			"movne	r2, #2				\n\t"
			/* restore user context saved by software */
			"add	r3, sp, #16			\n\t"
			"ldmia	r3, {r4-r11}			\n\t"
#ifdef CONFIG_FPU
			/* we don't use fpu in isr and syscall so don't need to
			 * restore it */
			"ldr	r3, [sp, #48]			\n\t"
			"tst	r3, #0x10			\n\t"
			"itt	eq				\n\t"
			"addeq	sp, sp, #72			\n\t"
			"orreq	r2, #4				\n\t"
#endif
			"msr	control, r2			\n\t"
			"add	sp, sp, #84			\n\t"
			"bx	r1				\n\t"
			:: "I"(TF_SYSCALL), "I"(TF_PRIVILEGED)
			: "r0", "r1", "r2", "r3", "lr", "memory");
}

/** run the handler in thread mode, allowing blocking operations including lock */
void syscall_delegate(void *func, void *sp, void *flags)
{
	__asm__ __volatile__(
			"mrs	r3, psp				\n\t"
			/* update task->mm.sp */
			"sub	r3, #16				\n\t"
			"str	r3, [r1]			\n\t"
			/* save flags and handler */
			"str	r0, [r3]			\n\t" /* handler */
			"str	r2, [r3, #4]			\n\t" /* flags(addr) */
			"ldr	r0, [r2]			\n\t"
			"str	r0, [r3, #8]			\n\t" /* flags */
			/* mark TF_SYSCALL in the task */
			"orr	r0, %0				\n\t"
			"str	r0, [r2]			\n\t"
			/* preserve space for copying context */
			"sub	r1, r3, #32			\n\t"
			"msr	psp, r1				\n\t"
			/* copy & manipulate context saved by hardware */
			"ldr	r2, [r3, #80]			\n\t" /* psr */
			"str	r2, [r1, #28]			\n\t"
			"ldr	r2, =syscall_delegate_callback	\n\t" /* pc */
			"str	r2, [r1, #24]			\n\t"
			/* give the task privilege */
			"mov	r0, #2				\n\t"
			"msr	control, r0			\n\t"
			"bx	lr				\n\t"
			:: "I"(TF_SYSCALL | TF_PRIVILEGED)
			: "r0", "r1", "r2", "r3", "lr", "memory");

	(void)(int)func;
	(void)(int)sp;
	(void)(int)flags;
}
#endif /* CONFIG_SYSCALL */
