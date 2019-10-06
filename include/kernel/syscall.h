#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "arch/hw_syscall.h"

#include <stdlib.h>
#include <stdint.h>

enum {
	SYSCALL_RESERVED	= 0,
	SYSCALL_TEST,
	SYSCALL_YIELD,
	SYSCALL_WRITE,
	SYSCALL_WAIT,
	SYSCALL_WAKE,		/* 5 */
	SYSCALL_SYSTICK,
	SYSCALL_TIMER_CREATE,
	SYSCALL_TIMER_DELETE,
	SYSCALL_TIMER_NEAREST,
	SYSCALL_REBOOT,		/* 10 */
	SYSCALL_NR,
};

int reboot(size_t msec);
int yield(void);
uint64_t get_systick64(void);
int timer_create(uint32_t interval_ticks, void (*cb)(void), uint8_t run);
int timer_delete(int timerid);
int32_t timer_nearest(void);

#endif /* __SYSCALL_H__ */
