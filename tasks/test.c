#include <foundation.h>
#include <kernel/task.h>

static void test_task()
{
	unsigned long long t;
	unsigned th, bh;
	unsigned v;

	printf("test()\n");
	printf("sp : %x, lr : %x\n", GET_SP(), GET_LR());

	while (1) {
		t = get_jiffies_64();
		th = t >> 32;
		bh = t;

		dmb();
		v = jiffies;

		printf("%08x ", v);
		printf("%08x", th);
		printf("%08x %d (%d sec)\n", bh, v, v/HZ);

		//reset_task_state(current, TASK_RUNNING);
		//schedule();
		sleep(1);
	}
}
REGISTER_TASK(test_task, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);
