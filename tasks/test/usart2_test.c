#include <foundation.h>
#include <kernel/task.h>

static void test_usart2()
{
	int fd;

	if ((fd = open("/dev/usart2", O_RDWR | O_NONBLOCK)) <= 0) {
		printf("usart2: open error %x\n", fd);
		return;
	}

	int buf, ret;

	while (1) {
		if ((ret = read(fd, &buf, 1)))
			putchar(buf);
	}
}
//REGISTER_TASK(test_usart2, 0, DEFAULT_PRIORITY);
