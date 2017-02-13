#include <foundation.h>
#include <kernel/task.h>
#include <kernel/timer.h>

static void test_gpio()
{
	unsigned int v = 0;
	int fd;

	if ((fd = open("/dev/gpio50", O_WRONLY)) <= 0) {
		printf("led open error %x\n", fd);
		return;
	}

	while (1) {
		write(fd, &v, 1);
		v ^= 1;
		sleep(1);
	}
}
REGISTER_TASK(test_gpio, TASK_PRIVILEGED, DEFAULT_PRIORITY);
