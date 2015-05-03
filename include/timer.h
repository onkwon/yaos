#ifndef __TIMER_H__
#define __TIMER_H__

#include <kernel/jiffies.h>

extern inline void sleep(unsigned long sec);
extern inline void msleep(unsigned long ms);

#endif /* __TIMER_H__ */
