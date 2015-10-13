#ifndef __ARMv7M_SYSCALL_H__
#define __ARMv7M_SYSCALL_H__

#define svc(n) \
	__asm__ __volatile__("svc %0" :: "I"(n) : "memory")

static int __attribute__((naked)) syscall(int n, ...)
{
	register int result __asm__("r0");
	svc(SYSCALL_NR);
	__asm__ __volatile__("bx lr");
	return result;
}

#endif /* __ARMv7M_SYSCALL_H__ */
