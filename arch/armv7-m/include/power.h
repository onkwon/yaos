#ifndef __ARMv7M_POWER_H__
#define __ARMv7M_POWER_H__

#define __wfi()				__asm__ __volatile__("wfi" ::: "memory")
#define __wfe()				__asm__ __volatile__("wfe" ::: "memory")

#define enter_sleep_mode()		__enter_sleep_mode()
#define enter_stop_mode()		__enter_stop_mode()
#define enter_standby_mode()		__enter_standby_mode()
#define sleep_on_exit()			__sleep_on_exit()

void __enter_sleep_mode();
void __enter_stop_mode();
void __enter_standby_mode();
void __sleep_on_exit();

#endif /* __ARMv7M_POWER_H__ */
