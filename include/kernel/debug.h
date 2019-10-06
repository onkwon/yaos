#ifndef __YAOS_DEBUG_H__
#define __YAOS_DEBUG_H__

#define TRAP_SYSCALL_OPEN		1
#define TRAP_SYSCALL_CLOSE		2
#define TRAP_SYSCALL_READ		3
#define TRAP_SYSCALL_LSEEK		4
#define TRAP_SYSCALL_FSTAT		5
#define TRAP_SYSCALL_ISATTY		6

#include "arch/hw_debug.h"

void debug_init(void);
void debug_putc(const int c);

void debug_print_context(uintptr_t *regs);
void debug_print_kernel_status(uintptr_t *sp, uintptr_t lr, uintptr_t psr);
void debug_print_user_status(uintptr_t *sp);
void debug_print_task_status(void *task);

#endif /* __YAOS_DEBUG_H__ */
