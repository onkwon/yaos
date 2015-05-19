#ifndef __JIFFIES_H__
#define __JIFFIES_H__

#define time_after(goal, chasing)	((long)goal    - (long)chasing < 0)
#define time_before(goal, chasing)	((long)chasing - (long)goal    < 0)

#define sec_to_jiffies(sec)	((sec) * HZ)
#define msec_to_jiffies(sec)	(sec_to_jiffies(sec) / 1000)

extern volatile unsigned long __attribute__((section(".data"))) jiffies;

unsigned long long get_jiffies_64();

#endif /* __JIFFIES_H__ */
