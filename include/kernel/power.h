#ifndef __POWER_H__
#define __POWER_H__

#include <asm/power.h>

extern int cpuload;

#define enter_stop_mode()		__enter_stop_mode()
#define enter_standby_mode()		__enter_standby_mode()
void enter_sleep_mode();

void reboot();

#endif /* __POWER_H__ */
