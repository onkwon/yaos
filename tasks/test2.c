#include "foundation.h"

#include "time.h"

static void test_task2()
{
	unsigned long long t;
	unsigned th, bh;

	while (1) {
		t = get_ticks_64();
		th = t >> 32;
		bh = t;

		printf("%08x ", ticks);
		printf("%08x", th);
		printf("%08x %d\n", bh, ticks);

		mdelay(500);
	}
}

#include "task.h"
REGISTER_TASK(test_task2, STACK_SIZE_DEFAULT, NORMAL_PRIORITY);
