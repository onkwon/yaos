#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <types.h>

#define SYSCALL_RESERVED		0
#define SYSCALL_SCHEDULE		1
#define SYSCALL_TEST			2
#define SYSCALL_OPEN			3
#define SYSCALL_READ			4
#define SYSCALL_WRITE			5
#define SYSCALL_CLOSE			6
#define SYSCALL_BRK			7
#define SYSCALL_YIELD			8
#define SYSCALL_MKNOD			9
#define SYSCALL_SEEK			10
#define SYSCALL_KILL			11
#define SYSCALL_FORK			12
#define SYSCALL_TIMER_CREATE		13
#define SYSCALL_SHUTDOWN		14
#define SYSCALL_IOCTL			15
#define SYSCALL_CREATE			16
#define SYSCALL_MKDIR			17
#define SYSCALL_NR			18

#define SYSCALL_DEFERRED_WORK		1

int sys_open(char *filename, int mode, void *opt);
int sys_read(int fd, void *buf, size_t len);
int sys_write(int fd, void *buf, size_t len);
int sys_ioctl(int fd, int request, void *data);
int sys_close(int fd);
int sys_shutdown(int option);

#ifdef CONFIG_SYSCALL
#ifdef MACHINE
#include <asm/syscall.h>
#endif
static inline int read(int fd, void *buf, size_t len)
{
	return syscall(SYSCALL_READ, fd, buf, len);
}
static inline int write(int fd, void *buf, size_t len)
{
	return syscall(SYSCALL_WRITE, fd, buf, len);
}
static inline int seek(int fd, unsigned int offset, int whence)
{
	return syscall(SYSCALL_SEEK, fd, offset, whence);
}
static inline int kill(unsigned int tid)
{
	return syscall(SYSCALL_KILL, tid);
}
static inline int fork()
{
	return syscall(SYSCALL_FORK);
}
#else
#define read(fd, buf, len)	sys_read(fd, buf, len)
#define write(fd, buf, len)	sys_write(fd, buf, len)
#endif /* CONFIG_SYSCALL */

int open(char *filename, int mode, ...);
int close(int fd);
int ioctl(int fd, int request, ...);
int shutdown(int option);

#endif /* __SYSCALL_H__ */
