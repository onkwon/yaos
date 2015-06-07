#include <foundation.h>
#include <kernel/task.h>

static void test_led()
{
	unsigned int v = 0;
	int fd;

	fd = open("/dev/led", 0);

	while (1) {
		write(fd, &v, 1);
		read(fd, &v, 1);
		printf("led %08x\n", v);
		v ^= 1;
		sleep(1);
	}
}
REGISTER_TASK(test_led, 0, DEFAULT_PRIORITY);
