#include <foundation.h>
#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/systick.h>
#include <stdlib.h>
#include <string.h>

static void test_clcd()
{
	int fd;
	if ((fd = open("/dev/clcd", O_RDWR)) <= 0) {
		printf("clcd: open error %x\n", fd);
		return;
	}

	unsigned int v;
	char *buf;

	if ((buf = malloc(128)) == NULL) {
		printf("malloc(): error\n");
		return;
	}

	while (1) {
		v = systick;
		sprintf(buf, "0x%08x %d (%d sec) ", v, v, v/HZ);
		write(fd, buf, strlen(buf));

		sleep(1);
	}

	write(fd, "Hello, World!", 13);
	sleep(5);
	write(fd, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
	stdout = fd;
}
//REGISTER_TASK(test_clcd, 0, DEFAULT_PRIORITY);
