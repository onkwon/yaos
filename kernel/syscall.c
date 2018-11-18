#include "log.h"

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

void *_sbrk(ptrdiff_t increment)
{
	(void)increment;
	debug("must not be called!");
	return NULL;
}

long _write(int fd, const void *buf, size_t cnt)
{
	(void)fd;
	(void)buf;
	(void)cnt;
	debug("must not be called!");
	return -EFAULT;
}

int _close(int fd)
{
	(void)fd;
	return -EFAULT;
}

int _fstat(int fd, struct stat *pstat)
{
	(void)fd;
	(void)pstat;
	debug("must not be called!");
	return -EFAULT;
}

int _isatty(int fd)
{
	(void)fd;
	debug("must not be called!");
	return -EFAULT;
}

off_t _lseek(int fd, off_t pos, int whence)
{
	(void)fd;
	(void)pos;
	(void)whence;
	debug("must not be called!");
	return -EFAULT;
}

long _read(int fd, void *buf, size_t cnt)
{
	(void)fd;
	(void)buf;
	(void)cnt;
	debug("must not be called!");
	return -EFAULT;
}

int _open(const char *pathname, int flags)
{
	(void)pathname;
	(void)flags;
	debug("must not be called!");
	return -EFAULT;
}
