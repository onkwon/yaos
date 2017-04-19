/*
 * [20:80]             (sp before system call)
 * [19:76] PSR         -
 * [18:72] PC          |
 * [17:68] LR          |
 * [16:64] R12         |
 * [15:60] R3(a2)      |- saved by hardware(8)
 * [14:56] R2(a1)      |
 * [13:52] R1(a0)      |
 * [12:48] R0(sysnum)  -
 * [11:44] R11         -
 * [10:40] R10         |
 * [09:36] R9          |
 * [08:32] R8          |
 * [07:28] R7          |- saved by software(8)
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

#include <kernel/task.h>

__attribute__((naked, noinline)) void syscall_delegate_callback()
{
	__asm__ __volatile__(
			/* arguments in place */
			"ldr	r0, [sp, #52]		\n\t" /* r1 */
			"ldr	r1, [sp, #56]		\n\t" /* r2 */
			"ldr	r2, [sp, #60]		\n\t" /* r3 */
			"ldr	r3, [sp]		\n\t" /* handler */
			"blx	r3			\n\t"
			/* restore user context saved by hardware */
			"ldr	r12, [sp, #64]		\n\t"
			"ldr	lr, [sp, #68]		\n\t"
			"ldr	r1, [sp, #72]		\n\t" /* pc */
			/* set LSB of return address */
			"orr	r1, #1			\n\t"
			/* back to the original state */
			"ldr	r2, [sp, #4]		\n\t" /* flags(addr) */
			"ldr	r3, [sp, #8]		\n\t" /* flags */
			"str	r3, [r2]		\n\t"
			"tst	r3, %1			\n\t"
			"itt	eq			\n\t"
			"moveq	r3, #3			\n\t"
			"msreq	control, r3		\n\t"
			/* restore user context saved by software */
			"add	r3, sp, #16		\n\t"
			"ldmia	r3, {r4-r11}		\n\t"
			"add	sp, sp, #80		\n\t"
			"bx	r1			\n\t"
			:: "I"(TF_SYSCALL), "I"(TF_PRIVILEGED)
			: "r0", "r1", "r2", "r3", "lr", "memory");
}

__attribute__((naked, noinline))
void syscall_delegate_atomic(void *func, void *sp, void *flags)
{
	__asm__ __volatile__(
			"mrs	r3, psp				\n\t"
			"sub	r3, r3, #48			\n\t"
			"msr	psp, r3				\n\t"
			/* update task->mm.sp */
			"add	r3, #32				\n\t"
			"str	r3, [r1]			\n\t"
			/* save flags and handler */
			"str	r0, [r3]			\n\t" /* handler */
			"str	r2, [r3, #4]			\n\t" /* flags(addr) */
			"ldr	r0, [r2]			\n\t"
			"str	r0, [r3, #8]			\n\t" /* flags */
			/* mark TF_SYSCALL in the task */
			"orr	r0, %0				\n\t"
			"str	r0, [r2]			\n\t"
			/* copy & manipulate context saved by hardware */
			"mov	r0, r3				\n\t"
			"ldr	r2, [r3, #76]!			\n\t" /* psr */
			"str	r2, [r0, #-4]!			\n\t"
			"ldr	r2, =syscall_delegate_callback	\n\t" /* pc */
			"str	r2, [r0, #-4]!			\n\t"
#if 0
			"ldr	r2, [r3, #-8]!			\n\t" /* lr */
			"str	r2, [r0, #-4]!			\n\t"
			"ldr	r2, [r3, #-4]!			\n\t" /* r12 */
			"str	r2, [r0, #-4]!			\n\t"
			"ldr	r2, [r3, #-4]!			\n\t" /* r3 */
			"str	r2, [r0, #-4]!			\n\t"
			"ldr	r2, [r3, #-4]!			\n\t" /* r2 */
			"str	r2, [r0, #-4]!			\n\t"
			"ldr	r2, [r3, #-4]!			\n\t" /* r1 */
			"str	r2, [r0, #-4]!			\n\t"
			"ldr	r2, [r3, #-4]!			\n\t" /* r0 */
			"str	r2, [r0, #-4]!			\n\t"
#endif
			/* give the task privilege */
			"mov	r0, #2				\n\t"
			"msr	control, r0			\n\t"
			"bx	lr				\n\t"
			:: "I"(TF_SYSCALL | TF_PRIVILEGED)
			: "r0", "r1", "r2", "r3", "lr", "memory");
}
