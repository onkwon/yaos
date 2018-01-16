#include <foundation.h>
#include <kernel/task.h>

static void test_uart2()
{
	int fd;

	if ((fd = open("/dev/uart2", O_RDWR | O_NONBLOCK)) <= 0) {
		printf("uart2: open error %x\n", fd);
		return;
	}

	int buf, ret;

	while (1) {
		if ((ret = read(fd, &buf, 1)))
			putchar(buf);
	}
}
//REGISTER_TASK(test_uart2, 0, DEFAULT_PRIORITY);
