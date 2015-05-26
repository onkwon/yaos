#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <types.h>

#ifdef CONFIG_SYSCALL
#define SYSCALL_RESERVED		0
#define SYSCALL_SCHEDULE		1
#define SYSCALL_TEST			2
#define SYSCALL_OPEN			3
#define SYSCALL_READ			4
#define SYSCALL_WRITE			5
#define SYSCALL_CLOSE			6
#define SYSCALL_BRK			7
#define SYSCALL_YIELD			8
#define SYSCALL_NR			9

#ifdef MACHINE
#include <asm/syscall.h>
#endif

static inline void yield()
{
	syscall(SYSCALL_YIELD);
}

static inline int open(int id, int mode)
{
	return syscall(SYSCALL_OPEN, id, mode);
}

static inline int read(int id, void *buf, size_t size)
{
	return syscall(SYSCALL_READ, id, buf, size);
}

static inline int write(int id, void *buf, size_t size)
{
	return syscall(SYSCALL_WRITE, id, buf, size);
}

static inline int close(int id)
{
	return syscall(SYSCALL_CLOSE, id);
}
#else
#include <kernel/task.h>
static inline void yield()
{
	set_task_state(current, TASK_SLEEPING);
	sys_schedule();
}

static inline int open(int id, int mode) { return 0; }
static inline int read(int id, void *buf, size_t size) { return 0; }
static inline int write(int id, void *buf, size_t size) { return 0; }
static inline int close(int id) { return 0; }
#endif /* CONFIG_SYSCALL */

#endif /* __SYSCALL_H__ */
