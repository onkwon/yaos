#include <foundation.h>

#include <time.h>
static void test_task2()
{
	unsigned long long t;
	unsigned th, bh;
	unsigned v;

	printf("test2()\n");
	printf("sp : %x, lr : %x\n", GET_SP(), GET_LR());

	while (1) {
		t = get_ticks_64();
		th = t >> 32;
		bh = t;

		dmb();
		v = ticks;

		printf("%08x ", v);
		printf("%08x", th);
		printf("%08x %d\n", bh, v);

		mdelay(500);
	}
}

#include <task.h>
REGISTER_TASK(test_task2, STACK_SIZE_DEFAULT, NORMAL_PRIORITY);
