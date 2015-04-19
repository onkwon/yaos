#include <foundation.h>

static void test_task1()
{
	while (1) {
		printf("test1()\n");
		mdelay(1000);
	}
}

#include <kernel/task.h>
REGISTER_TASK(test_task1, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);
