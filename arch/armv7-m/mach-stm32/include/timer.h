#ifndef __STM32_TIMER_H__
#define __STM32_TIMER_H__

#define NR_TIMER_MAX		4
#define TIM_COUNTER_MAX		(1 << 16) /* 65536 */

#define TIM_OVERCAPTURE_MASK	0x1e00
#define TIM_COMPARE_MASK	0x1e
#define TIM_UPDATE_MASK		0x1

struct __timer {
	unsigned int cr1;
	unsigned int cr2;
	unsigned int smcr;
	unsigned int dier;
	unsigned int sr;
	unsigned int egr;
	unsigned int ccmr1;
	unsigned int ccmr2;
	unsigned int ccer;
	unsigned int cnt;
	unsigned int psc;
	unsigned int arr;
	unsigned int reserved;
	unsigned int ccr1;
	unsigned int ccr2;
	unsigned int ccr3;
	unsigned int ccr4;
};

#include <io.h>

static inline bool check_timer_int_source(int flags, int mask)
{
	if (flags & mask)
		return true;

	return false;
}

static inline int get_timer_int_source_channel(int flags)
{
	return (flags & TIM_COMPARE_MASK) >> 1;
}

static inline int __get_irq_source_timer()
{
	int n = get_active_irq() - 42;

	if (n == 24)
		n = 5;

	return n;
}

static inline struct __timer *__timer_id2reg(int id)
{
	return (struct __timer *)(TIM2_BASE | ((id-2) << 10));
}

#include <timer.h>

int __timer_open(int id, bool dir, unsigned int hz);
void __timer_run(int id, bool on);
void __timer_mode_set(int id, enum tim_mode mode);
void __timer_channel_set(int id, enum tim_iochannel ch, enum tim_iomode iomode,
		int pin, bool dir);
void __timer_prescale_set(int id, unsigned int v);
void __timer_period_set(int id, unsigned int v);
void __timer_match_set(int id, enum tim_iochannel ch, unsigned int v);
void __timer_intr_set(int id, enum tim_iochannel ch);
void __timer_reload(int id);

#endif /* __STM32_TIMER_H__ */
