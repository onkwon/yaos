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
	svc(SYSCALL_NR);

	register int result __asm__("r0");
	return result;
}

#endif /* __ARMv7M_SYSCALL_H__ */
