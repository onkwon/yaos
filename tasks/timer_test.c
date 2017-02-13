#include <foundation.h>
#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/systick.h>

static volatile int flag;
static void t1()
{
	printf("TIMER1\n");
	flag = 1;
}

static void test_timer()
{
	struct timer ttt;
	while (1) {
		flag = 0;
		ttt.expires = systick + msec_to_ticks(500);
		ttt.event = t1;
		add_timer(&ttt);

		msleep(900);
		while (flag == 0);
	}
}
//REGISTER_TASK(test_timer, 0, DEFAULT_PRIORITY);
