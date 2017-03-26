#include <foundation.h>
#include <kernel/timer.h>

static void led()
{
	int fd, v = 0;
	const char *pathname = "/dev/led" def2str(PIN_STATUS_LED);

	if ((fd = open(pathname, O_WRONLY)) <= 0) {
		error("can not open %x\n", fd);
		return;
	}

	while (1) {
		v ^= 1;
		write(fd, &v, 1);
		sleep(1);
	}

	close(fd);
}
REGISTER_TASK(led, 0, DEFAULT_PRIORITY, STACK_SIZE_MIN);
