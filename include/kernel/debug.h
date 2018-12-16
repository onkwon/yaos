#ifndef __YAOS_DEBUG_H__
#define __YAOS_DEBUG_H__

#define TRAP_SYSCALL_OPEN		1
#define TRAP_SYSCALL_CLOSE		2
#define TRAP_SYSCALL_READ		3
#define TRAP_SYSCALL_LSEEK		4
#define TRAP_SYSCALL_FSTAT		5
#define TRAP_SYSCALL_ISATTY		6

#include "arch/hw_debug.h"

#define debug_putc			hw_debug_putc

void debug_init(void);

#endif /* __YAOS_DEBUG_H__ */
