#include "kernel/syscall.h"
#include "kernel/interrupt.h"
#include "kernel/debug.h"
#include "kernel/systick.h"
#include "kernel/timer.h"
#include "syslog.h"
#include "io.h"

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

void *_sbrk(ptrdiff_t increment);
long _write(int fd, const void *buf, size_t cnt);
int _close(int fd);
int _fstat(int fd, struct stat *pstat);
int _isatty(int fd);
off_t _lseek(int fd, off_t pos, int whence);
long _read(int fd, void *buf, size_t cnt);
int _open(const char *pathname, int flags);

extern uintptr_t _heap_start;

void *_sbrk(ptrdiff_t increment)
{
	static uintptr_t *brk = (uintptr_t *)&_heap_start;

	brk += increment / sizeof(uintptr_t);

	return brk;
}

static long sys_write(int fd, const void *buf, size_t cnt)
{
	const char *dat = (const char *)buf;
	void (*put)(const int c) = NULL;
	size_t i;

	switch (fd) {
	case SYSLOG_FD_DEBUG:
		put = debug_putc;
		break;
	case SYSLOG_FD_STDOUT:
	case SYSLOG_FD_BUFFER:
		if (printc)
			put = printc;
		break;
	default:
		break;
	}

	if (!put)
		return 0;

	for (i = 0; i < cnt; i++)
		put((int)dat[i]);

	return i;
}

long _write(int fd, const void *buf, size_t cnt)
{
	/* if not initialized yet or called from ISR like scheduler */
	if (is_interrupt_disabled() || in_interrupt())
		return sys_write(fd, buf, cnt);

	return syscall(SYSCALL_WRITE, fd, buf, cnt);
}

int _close(int fd)
{
	(void)fd;
	__trap(TRAP_SYSCALL_CLOSE);
	return -EFAULT;
}

int _fstat(int fd, struct stat *pstat)
{
	(void)fd;
	(void)pstat;
	return -EFAULT;
#if 0
	pstat->st_mode = S_IFCHR;

	__trap(TRAP_SYSCALL_FSTAT);
	return 0;
#endif
}

int _isatty(int fd)
{
	(void)fd;
	__trap(TRAP_SYSCALL_ISATTY);
	return -EFAULT;
}

off_t _lseek(int fd, off_t pos, int whence)
{
	(void)fd;
	(void)pos;
	(void)whence;
	__trap(TRAP_SYSCALL_LSEEK);
	return -EFAULT;
}

long _read(int fd, void *buf, size_t cnt)
{
	(void)fd;
	(void)buf;
	(void)cnt;
	__trap(TRAP_SYSCALL_READ);
	return -EFAULT;
}

int _open(const char *pathname, int flags)
{
	(void)pathname;
	(void)flags;
	__trap(TRAP_SYSCALL_OPEN);
	return -EFAULT;
}

int reboot(unsigned long msec)
{
	mdelay(msec);

	hw_reboot();

	return 0;
}

int yield(void)
{
	return syscall(SYSCALL_YIELD);
}

uint64_t get_systick64(void)
{
	return syscall(SYSCALL_SYSTICK);
}

int timer_create(uint32_t interval_ticks, void (*cb)(void), uint8_t run)
{
	return syscall(SYSCALL_TIMER_CREATE, interval_ticks, cb, run);
}

int timer_delete(int timerid)
{
	return syscall(SYSCALL_TIMER_DELETE, timerid);
}

int32_t timer_nearest(void)
{
	return syscall(SYSCALL_TIMER_NEAREST);
}

static int sys_reserved(void)
{
	return -EFAULT;
}

static int sys_test(int a, int b)
{
	debug("a + b = %d", a + b);
	return 0;
}

#include "kernel/sched.h"

void *syscall_table[] = {
	sys_reserved,			/*  0: SYSCALL_RESERVED */
	sys_test,			/*  1: SYSCALL_TEST */
	sched_yield,			/*  2: SYSCALL_YIELD */
	sys_write,			/*  3: SYSCALL_WRITE */
	task_wait,			/*  4: SYSCALL_WAIT */
	task_wake,			/*  5: SYSCALL_WAKE */
	get_systick64_core,		/*  6: SYSCALL_SYSTICK */
	timer_create_core,		/*  7: SYSCALL_TIMER_CREATE */
	timer_delete_core,		/*  8: SYSCALL_TIMER_DELETE */
	timer_nearest_core,		/*  8: SYSCALL_TIMER_NEAREST */
};
