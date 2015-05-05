#include <foundation.h>
#include <kernel/task.h>

static void test_task1()
{
	while (1) {
		printf("test1()\n");
		sleep(1);
	}
}
REGISTER_TASK(test_task1, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);
