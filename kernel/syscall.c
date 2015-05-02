#include <syscall.h>

int sys_reserved();
extern int sys_schedule();
extern int sys_test(int a, int b, int c);

void *syscall_table[] = {
	sys_reserved,		/* 0 */
	sys_schedule,
	sys_test,
	sys_reserved,
	sys_reserved,
	sys_reserved,		/* 5 */
};

int syscall(int n, ...)
{
	svc(SYSCALL_NR);
}

int sys_reserved()
{
	return -SYSCALL_UNDEF;
}
