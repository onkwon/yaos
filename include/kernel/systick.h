#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#include <types.h>

#define time_after(goal, chasing)	((int)(goal)    - (int)(chasing) < 0)
#define time_before(goal, chasing)	((int)(chasing) - (int)(goal)    < 0)

#define sec_to_ticks(sec)		((sec) * sysfreq)
#define msec_to_ticks(sec)		(sec_to_ticks(sec) / 1000)

extern unsigned int sysfreq;
extern volatile unsigned int __attribute__((section(".data"))) systick;

uint64_t get_systick64();

void systick_init();

#endif /* __SYSTICK_H__ */
