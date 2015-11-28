#ifndef __ARMv7M_SYSCALL_H__
#define __ARMv7M_SYSCALL_H__

#define svc(n) \
	__asm__ __volatile__("svc	%0	\n\t"	\
			     "bx	lr	\n\t"	\
			:: "I"(n) : "memory")

static int __attribute__((naked)) syscall(int n, ...)
{
	svc(SYSCALL_NR);

	register int result __asm__("r0");
	return result;
}

#endif /* __ARMv7M_SYSCALL_H__ */
