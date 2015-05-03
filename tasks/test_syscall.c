#include <foundation.h>
#include <syscall.h>

int sys_test(int a, int b, int c)
{
	printf("%d + %d + %d\n", a, b, c);
	return a + b + c;
}

static void test_syscall()
{
	while (1) {
		printf("syscall : %08x\n", syscall(SYSCALL_TEST, 1, 2, 3));
		sleep(5);
	}
}

REGISTER_TASK(test_syscall, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);
