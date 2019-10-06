#ifndef __YAOS_ARMv7M_HW_SYSCALL_H__
#define __YAOS_ARMv7M_HW_SYSCALL_H__

#define svc(n)					\
	__asm__ __volatile__(			\
			"svc	%0	\n\t"	\
			"bx	lr	\n\t"	\
			:: "I"(n) : "memory")

/* It must not be an inline function as the first parameter, `r0`, is used to
 * pass syscall number to distinguish each syscalls */
int __attribute__((naked, noinline, used)) syscall(int n, ...);

void __attribute__((naked, noinline)) syscall_delegate(void *func, void *sp, void *flags);
void __attribute__((naked, noinline)) syscall_delegate_callback(void);

#endif /* __YAOS_ARMv7M_HW_SYSCALL_H__ */
