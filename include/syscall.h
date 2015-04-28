#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define SYSCALL_RESERVED		0
#define SYSCALL_SCHEDULE		1
#define SYSCALL_TEST			2
#define SYSCALL_NR			3

#ifdef MACHINE
#include <asm/syscall.h>
#endif

#define SYSCALL_UNDEF			1

int syscall(int n, ...);

#endif /* __SYSCALL_H__ */
