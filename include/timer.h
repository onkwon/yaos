#ifndef __TIMER_H__
#define __TIMER_H__

#define TIM_DIR_SHIFT	4
#define TIM_OP_MASK	((1 << 4) - 1)

enum tim_mode {
	TIM_OP_EDGE	= 0x00,
	TIM_OP_CENTER	= 0x01,
	TIM_OP_CENTER2	= 0x02,
	TIM_OP_CENTER3	= 0x03,
	TIM_MODE_MAX	= TIM_OP_CENTER3,
	TIM_OP_UPCOUNT	= TIM_OP_EDGE,
	TIM_OP_DNCOUNT	= 0x10,
};

enum tim_iomode {
	TIM_IO_FROZEN	= 0,
	TIM_IO_LO2HI	= 1,
	TIM_IO_HI2LO	= 2,
	TIM_IO_TOGGLE	= 3,
	TIM_IO_PWM	= 6,
	TIM_IO_PWM2	= 7,
	TIM_IOMODE_MAX,
};

enum tim_iochannel {
	TIM_IO_NONE	= 0,
	TIM_IO_OVERFLOW	= TIM_IO_NONE,
	TIM_IO_CH1	= 1,
	TIM_IO_CH2	= 2,
	TIM_IO_CH3	= 3,
	TIM_IO_CH4	= 4,
	TIM_CHANNEL_MAX,
};

#include <types.h>

struct gtimer {
	enum tim_mode mode;
	enum tim_iochannel channel;
	unsigned int prescale;
	unsigned int period;
	unsigned int match;
	enum tim_iomode iomode;
	bool interrupt;
	int (*isr)(int flags);
	int pin;
};

typedef struct gtimer timer_t;
#endif /* __TIMER_H__ */
