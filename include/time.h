#ifndef __TIME_H__
#define __TIME_H__

extern unsigned long volatile __attribute__((section(".data"))) ticks;

unsigned long long get_ticks_64();

void inline update_curr(unsigned elapsed);

#endif /* __TIME_H__ */
