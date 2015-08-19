#ifndef __TICKS_H__
#define __TICKS_H__

#include <types.h>

#define time_after(goal, chasing)	((int)goal    - (int)chasing < 0)
#define time_before(goal, chasing)	((int)chasing - (int)goal    < 0)

#define sec_to_ticks(sec)		((sec) * HZ)
#define msec_to_ticks(sec)		(sec_to_ticks(sec) / 1000)

extern volatile unsigned int __attribute__((section(".data"))) ticks;

uint64_t get_ticks_64();

#endif /* __TICKS_H__ */
