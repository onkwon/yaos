#include <foundation.h>
#include <kernel/task.h>

static void test_led()
{
	extern int led_id;
	unsigned int v = 0;

	while (1) {
		write(led_id, &v, 1);
		read(led_id, &v, 1);
		printf("led %08x\n", v);
		v ^= 1;
		sleep(2);
	}
}
REGISTER_TASK(test_led, 0, DEFAULT_PRIORITY);
