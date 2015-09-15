#ifndef __RPI_SYSCALL_H__
#define __RPI_SYSCALL_H__

#include <asm/interrupt.h>

#define svc(n) \
	__asm__ __volatile__("svc %0" :: "I"(n) : "memory")

static int __attribute__((naked)) syscall(int n, ...)
{
	register int result __asm__("r0");
	svc(SYSCALL_NR);
	__ret();
	return result;
}

#endif /* __RPI_SYSCALL_H__ */
