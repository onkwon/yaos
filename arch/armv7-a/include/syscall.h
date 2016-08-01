#ifndef __ARMv7A_SYSCALL_H__
#define __ARMv7A_SYSCALL_H__

#define svc(n) \
	__asm__ __volatile__("svc	%0	\n\t"	\
			     "bx	lr	\n\t"	\
			:: "I"(n) : "memory")

static int __attribute__((naked, noinline)) syscall(int n, ...)
{
	svc(SYSCALL_NR);

	register int result __asm__("r0");
	return result;
}

extern void __reboot();
extern int sys_reboot(); /* sys_reboot.c */

#endif /* __ARMv7A_SYSCALL_H__ */
