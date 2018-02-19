#include <foundation.h>
#include <kernel/timer.h>

#define MAXLEN	(33*2)

static void test_ir()
{
	int fd, i, len;
	int buf[MAXLEN];
	unsigned int val, sum;

	if ((fd = open("/dev/ir", O_RDONLY)) <= 0) {
		error("can not open %d\n", fd);
		return;
	}

	while (1) {
		if ((len = read(fd, buf, MAXLEN)) <= 0 || len < MAXLEN)
			continue;

#if 1
		for (i = 0; i < len; i++) {
			if (!(i % 8))
				putchar('\n');
			printf("%7d ", buf[i]);
		}
#endif

		val = 0;

		for (i = 0; i < len; i += 2) {
			sum = buf[i] + buf[i+1];
			if (sum > 2500) // header
				;
			else if (sum > 1500) // high
				val = (val << 1) | 1;
			else // low
				val = (val << 1);
		}
		printf("\n0x%08x\n", val);
	}
}
REGISTER_TASK(test_ir, TASK_KERNEL, DEFAULT_PRIORITY, STACK_SIZE_DEFAULT);
