/*
 * [19:76]             (sp before system call)
 * [18:72] PSR         -
 * [17:68] PC          |
 * [16:64] LR          |
 * [15:60] R12         |
 * [14:56] R3          |- saved by hardware
 * [13:52] R2          |
 * [12:48] R1          |
 * [11:44] R0          -
 * [10:40] R11         -
 * [09:36] R10         |
 * [08:32] R9          |
 * [07:28] R8          |
 * [06:24] R7          |- saved by software
 * [05:20] R6          |
 * [04:16] R5          |
 * [03:12] R4          -
 * [02:08] flags
 * [01:04] flags(addr)
 * [00:00] handler     (sp when exit from system call)
 *         PSR         -
 *         PC(callback)|
 *         LR          |
 *         R12         |
 *         R3          |- copied from above
 *         R2          |
 *         R1          |
 *         R0          -
 */

#include <kernel/task.h>

__attribute__((naked, noinline)) void syscall_delegate_callback()
{
	__asm__ __volatile__(
			/* arguments in place */
			"ldr	r0, [sp, #48]		\n\t"
			"ldr	r1, [sp, #52]		\n\t"
			"ldr	r2, [sp, #56]		\n\t"
			/* set return address */
			"ldr	lr, =1f			\n\t"
			"orr	lr, #1			\n\t"
			/* jump to handler */
			"pop	{pc}			\n\t"
			/* restore user context saved by hardware */
		"1:"	"ldr	r12, [sp, #56]		\n\t"
			"ldr	lr, [sp, #60]		\n\t"
			"ldr	r1, [sp, #64]		\n\t" /* pc */
			/* set LSB of return address */
			"orr	r1, #1			\n\t"
			/* back to the original state */
			"pop	{r2}			\n\t" /* flags(addr) */
			"pop	{r3}			\n\t" /* flags */
			"str	r3, [r2]		\n\t"
			"tst	r3, %1			\n\t"
			"itt	eq			\n\t"
			"moveq	r3, #3			\n\t"
			"msreq	control, r3		\n\t"
			/* restore user context saved by software */
			"pop	{r4-r11}		\n\t"
			"add	sp, #32			\n\t"
			"bx	r1			\n\t"
			:: "I"(TF_SYSCALL), "I"(TF_PRIVILEGED)
			: "r0", "r1", "r2", "r3", "lr", "memory");
}

__attribute__((naked, noinline))
void syscall_delegate_atomic(void *func, void *sp, void *flags)
{
	__asm__ __volatile__(
			/* save flags and handler */
			"mrs	r3, psp				\n\t"
			"str	r0, [r3, #-12]			\n\t" /* handler */
			"str	r2, [r3, #-8]			\n\t" /* flags(addr) */
			"ldr	r0, [r2]			\n\t"
			"str	r0, [r3, #-4]			\n\t" /* flags */
			"sub	r3, #12				\n\t"
			/* update sp in task structure */
			"str	r3, [r1]			\n\t"
			/* mark TF_SYSCALL in the task */
			"orr	r0, %0				\n\t"
			"str	r0, [r2]			\n\t"
			/* copy & manipulate context saved by hardware */
			"mov	r0, r3				\n\t"
			"ldr	r2, [r3, #72]!			\n\t" /* psr */
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
#else
			"sub	r0, #24				\n\t"
#endif
			"msr	psp, r0				\n\t"
			/* give the task privilege */
			"mov	r0, #2				\n\t"
			"msr	control, r0			\n\t"
			"bx	lr				\n\t"
			:: "I"(TF_SYSCALL | TF_PRIVILEGED)
			: "r0", "r1", "r2", "r3", "lr", "memory");
}
