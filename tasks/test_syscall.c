#include <foundation.h>
#include <syscall.h>

static void test_syscall()
{
	while (1) {
		printf("syscall : %08x\n", syscall(SYSCALL_TEST, 1, 2, 3));
		sleep(5);
	}
}
REGISTER_TASK(test_syscall, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);

#include <kernel/device.h>
static void test_driver()
{
	char t = 'S';
	syscall(SYSCALL_OPEN, USART);
	while (1) {
		printf("DRIVER %d ", syscall(SYSCALL_READ, USART, &t, 1));
		printf("%c ,", t);
		printf("%d ", syscall(SYSCALL_WRITE, USART, &t, 1));
		printf("%c \n", t);
		sleep(3);
	}
}
REGISTER_TASK(test_driver, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);
