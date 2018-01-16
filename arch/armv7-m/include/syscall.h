#ifndef __ARMv7M_SYSCALL_H__
#define __ARMv7M_SYSCALL_H__

#define svc(n) \
	__asm__ __volatile__("svc	%0	\n\t"	\
			     "bx	lr	\n\t"	\
			:: "I"(n) : "memory")

/* It must not be an inline function as the first parameter, `r0`, is used to
 * pass syscall number to distinguish each syscalls */
static int __attribute__((naked, noinline)) syscall(int n, ...)
{
	__asm__ __volatile__(
			"svc	%0	\n\t"
			"bx	lr	\n\t"
			:: "I"(SYSCALL_NR) : "memory");

	register int result __asm__("r0");

	(void)n;
	return result;
}

void syscall_delegate_callback();
void syscall_delegate_atomic(void *func, void *sp, void *flag);

#endif /* __ARMv7M_SYSCALL_H__ */
