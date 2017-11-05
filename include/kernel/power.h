#ifndef __POWER_H__
#define __POWER_H__

#include <asm/power.h>

extern int cpuload;

void enter_sleep_mode(sleep_t sleeptype);
void reboot();

#endif /* __POWER_H__ */
