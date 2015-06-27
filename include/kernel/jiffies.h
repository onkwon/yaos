#ifndef __JIFFIES_H__
#define __JIFFIES_H__

#include <types.h>

#define time_after(goal, chasing)	((int)goal    - (int)chasing < 0)
#define time_before(goal, chasing)	((int)chasing - (int)goal    < 0)

#define sec_to_jiffies(sec)	((sec) * HZ)
#define msec_to_jiffies(sec)	(sec_to_jiffies(sec) / 1000)

extern volatile unsigned int __attribute__((section(".data"))) jiffies;

uint64_t get_jiffies_64();
extern inline void update_tick(unsigned delta);

#endif /* __JIFFIES_H__ */
