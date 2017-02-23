#include <foundation.h>
#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/systick.h>

#if 0 /* kernel timer test */
static volatile int flag;
static void t1()
{
	printf("TIMER1\n");
	flag = 1;
}

static void test_timer()
{
	struct ktimer ttt;
	while (1) {
		flag = 0;
		ttt.expires = systick + msec_to_ticks(500);
		ttt.event = t1;
		add_timer(&ttt);

		msleep(900);
		while (flag == 0);
	}
}
REGISTER_TASK(test_timer, 0, DEFAULT_PRIORITY);
#else
#include <timer.h>
#include <asm/pinmap.h>
#include <stdlib.h>

#include <asm/timer.h>
static void test_timer()
{
	int fd;
	timer_t tim;

	fd = open("/dev/tim3", O_WRONLY);
	memset(&tim, 0, sizeof(tim));
	tim.channel = TIM_IO_CH1;
	tim.pin = PIN_TIM3CH1;
	tim.iomode = TIM_IO_PWM;
	//tim.prescale = 8;
	tim.prescale = 1124;
	//tim.period = 63999;
	tim.period = 31999;
	tim.match = tim.period / 2;
	ioctl(fd, C_SET, &tim);

	while (1) {
		tim.match += 100;
		if (tim.match > tim.period)
			tim.match = 1;

		ioctl(fd, C_RUN, &tim);
		sleep(1);
	}

	close(fd);
}
REGISTER_TASK(test_timer, TASK_KERNEL, DEFAULT_PRIORITY);
#endif
