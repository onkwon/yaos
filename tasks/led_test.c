#include <foundation.h>
#include <kernel/task.h>
#include <kernel/timer.h>

#include <asm/pinmap.h>

#ifdef PIN_STATUS_LED
static void test_led()
{
	unsigned int v = 0;
	int fd;

	if ((fd = open("/dev/led", O_RDWR, PIN_STATUS_LED)) <= 0) {
		printf("led open error %x\n", fd);
		return;
	}

	while (1) {
		write(fd, &v, 1);
		read(fd, &v, 1);
		printf("led %08x\n", v);
		v ^= 1;
		sleep(1);
	}
}
//REGISTER_TASK(test_led, 0, DEFAULT_PRIORITY);
#endif
