#include "syslog.h"
#include "kernel/interrupt.h"
#include "kernel/debug.h"

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

long _write(int fd, const void *buf, size_t cnt)
{
	const char *dat = (const char *)buf;

	for (size_t i = 0; i < cnt; i++)
		debug_putc((int)dat[i]);

	return cnt;
	(void)fd;
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
