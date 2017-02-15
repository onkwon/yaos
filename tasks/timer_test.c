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

static volatile int cc1, cc2, flag;
static int test_isr(int flags)
{
	cc1 = TIM2_CCR1;
	cc2 = TIM2_CCR2;
	flag = 1;

	return 0;
}

#include <asm/timer.h>
static void test_timer()
{
	int fd2, fd3;
	timer_t tim;
	int psc = 0xffff;

	/* tim2 */
	fd2 = open("/dev/tim2", O_RDONLY);
	memset(&tim, 0, sizeof(tim));
	tim.channel = TIM_IO_CH2;
	tim.pin = PIN_TIM2CH2;
	tim.iomode = TIM_IO_PWM;
	tim.interrupt = true;
	tim.isr = test_isr;
tim.prescale = psc - 1;
	ioctl(fd2, C_SET, &tim);

	/* tim3 */
	fd3 = open("/dev/tim3", O_WRONLY);
	memset(&tim, 0, sizeof(tim));
	tim.channel = TIM_IO_CH1;
	tim.pin = PIN_TIM3CH1;
	tim.iomode = TIM_IO_PWM;
	//tim.prescale = 8;
	tim.prescale = 1124;
	//tim.period = 63999;
	tim.period = 31999;
	tim.match = tim.period / 2;
	ioctl(fd3, C_SET, &tim);

	volatile int ccr, t1, t2;
	int r[4];

	while (1) {
#if 0
		if (flag) {
			ccr = tim.match;
			t1 = cc1;
			t2 = cc2;

			printf("%d : %d %d f=%dHz %3d%%\n", ccr, t1, t2,
					72000000/psc/t2, t1*100/t2);

			if (++ccr > tim.period)
				ccr = 1;

			tim.match = ccr;
			ioctl(fd3, C_RUN, &tim);
			//msleep(200);
			flag = 0;
		}
#else
		//if (has_event(fd2)) {
			if (!read(fd2, r, sizeof(r)))
				continue;
			ccr = tim.match;
			t1 = r[0];
			t2 = r[1];

			printf("%d : %d %d f=%dHz %3d%%\n", ccr, t1, t2,
					72000000/psc/t2, t1*100/t2);

			ccr += 100;
			if (ccr > tim.period)
				ccr = 1;

			tim.match = ccr;
			ioctl(fd3, C_RUN, &tim);
		//}
#endif
	}

	close(fd2);
	close(fd3);
}
REGISTER_TASK(test_timer, TASK_KERNEL, DEFAULT_PRIORITY);
#endif
