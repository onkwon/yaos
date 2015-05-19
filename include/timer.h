#ifndef __TIMER_H__
#define __TIMER_H__

#include <kernel/jiffies.h>

extern inline void sleep(unsigned long sec);
extern inline void msleep(unsigned long ms);

extern inline unsigned long set_timeout(unsigned long ms);
extern inline int is_timeout(unsigned long goal);

#endif /* __TIMER_H__ */
