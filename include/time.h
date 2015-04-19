#ifndef __TIME_H__
#define __TIME_H__

extern unsigned long volatile __attribute__((section(".data"))) systick;

unsigned long long get_systick_64();

#endif /* __TIME_H__ */
