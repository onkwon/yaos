#include "foundation.h"

static void test_task2()
{
	while (1) {
		printf("test2()\n");
		mdelay(500);
	}
}

#include "task.h"
REGISTER_TASK(test_task2, STACK_SIZE_DEFAULT, NORMAL_PRIORITY);
