#ifndef __ARMv7M_POWER_H__
#define __ARMv7M_POWER_H__

#include <types.h>

#define __wfi()				__asm__ __volatile__("wfi" ::: "memory")
#define __wfe()				__asm__ __volatile__("wfe" ::: "memory")

#define sleep_on_exit()			__sleep_on_exit()

void __enter_sleep_mode();
void __enter_stop_mode();
void __enter_standby_mode();
void __sleep_on_exit();
void __set_power_regulator(bool on, int scalemode, bool overdrive);
sleep_t get_sleep_type();

void __reboot();

unsigned int __read_reset_source();

#endif /* __ARMv7M_POWER_H__ */
