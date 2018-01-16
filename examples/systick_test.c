#include <foundation.h>
#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/systick.h>

static void test_task()
{
	unsigned long long t;
	unsigned int th, bh;
	unsigned int v;

	while (1) {
		t = get_systick64();
		th = t >> 32;
		bh = t;

		dmb();
		v = systick;

		printf("%08x ", v);
		printf("%08x", th);
		printf("%08x %d (%d sec)\n", bh, v, v/HZ);

		sleep(1);
	}
}
REGISTER_TASK(test_task, 0, DEFAULT_PRIORITY);
