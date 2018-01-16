#include <foundation.h>
#include <kernel/task.h>
#include <kernel/timer.h>

static void mayfly()
{
	printf("onetime function\n");

	sleep(5);
	printf("REBOOT %x\n", shutdown(2));
}
//REGISTER_TASK(mayfly, 0, DEFAULT_PRIORITY); /* must be previleged */
