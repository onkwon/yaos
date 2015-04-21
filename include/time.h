#ifndef __TIME_H__
#define __TIME_H__

extern unsigned long volatile __attribute__((section(".data"))) jiffies;

unsigned long long get_jiffies_64();

#endif /* __TIME_H__ */
