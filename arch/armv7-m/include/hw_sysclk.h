#ifndef __YAOS_HW_SYSCLK_H__
#define __YAOS_HW_SYSCLK_H__

#define HW_SYSCLK_MAXFREQ_KHZ		(100U)
#define HW_SYSCLK_RESOLUTION		((1UL << 24) - 1) /* 24-bit timer */

void hw_sysclk_reset(void);
void hw_sysclk_period_set(unsigned long period);
unsigned long hw_sysclk_period_get(void);
void hw_sysclk_run(void);
void hw_sysclk_stop(void);
unsigned long hw_sysclk_freq_get(void);

#endif /* __YAOS_HW_SYSCLK_H__ */
