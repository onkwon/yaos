#include <foundation.h>
#include <kernel/task.h>
#include <timer.h>
#include <asm/pinmap.h>
#include <stdlib.h>

#include "pcm.h"

volatile bool update;

static int myisr(int flags)
{
	update = true;
	return 0;
}

static void pcm_test()
{
	int fd, fd2, i = 0;
	timer_t tim;

	fd = open("/dev/tim2", O_WRONLY, 16000);
	memset(&tim, 0, sizeof(tim));
	tim.interrupt = true;
	tim.isr = myisr;
	ioctl(fd, C_SET, &tim);

	fd2 = open("/dev/tim3", O_WRONLY);
	memset(&tim, 0, sizeof(tim));
	tim.channel = TIM_IO_CH1;
	tim.pin = PIN_TIM3CH1;
	tim.iomode = TIM_IO_PWM;
	//tim.prescale = 8;
	tim.period = 255;
	ioctl(fd2, C_SET, &tim);

	while (1) {
		if (update) {
			tim.match = pcm[i];
			ioctl(fd2, C_RUN, &tim);
			if (++i >= pcm_len)
				i = 0;
			update = 0;
		}
	}
}
//REGISTER_TASK(pcm_test, TASK_KERNEL, DEFAULT_PRIORITY);
