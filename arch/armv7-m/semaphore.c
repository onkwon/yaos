#include <kernel/lock.h>

void semaphore_dec(struct semaphore *sem)
{
	int count, result;

	/* keep constraints `h` not `r` to ensure coherence of the registers,
	 * not like the registers `r0-r3` affected by callee. */
	__asm__ __volatile__(
		"1:"	"ldrex	%0, [%2]		\n\t"
			"cmp	%0, #0			\n\t"
			"bgt	2f			\n\t"
			"mov	r0, %3			\n\t"
			"bl	sleep_in_waitqueue	\n\t"
			"b	1b			\n\t"
		"2:"	"sub	%0, #1			\n\t"
			"strex	%1, %0, [%2]		\n\t"
			"cmp	%1, #0			\n\t"
			"bne	1b			\n\t"
			"dmb				\n\t"
			: "=&h"(count), "=&h"(result)
			: "h"(&sem->counter), "h"(&sem->wq)
			: "r0", "lr", "cc", "memory");
}

void semaphore_inc(struct semaphore *sem)
{
	int count, result;

	__asm__ __volatile__(
		"1:"	"ldrex	%0, [%2]		\n\t"
			"add	%0, #1			\n\t"
			"strex	%1, %0, [%2]		\n\t"
			"cmp	%1, #0			\n\t"
			"bne	1b			\n\t"
			"cmp	%0, #0			\n\t"
			"dmb				\n\t"
			"itt	gt			\n\t"
			"movgt	r0, %3			\n\t"
			"blgt	shake_waitqueue_out	\n\t"
			: "=&h"(count), "=&h"(result)
			: "h"(&sem->counter), "h"(&sem->wq)
			: "r0", "lr", "cc", "memory");
}
