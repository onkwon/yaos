#include <kernel/lock.h>

unsigned int sysfreq;

volatile unsigned int __attribute__((section(".data"))) systick, systick_ms;
uint64_t __attribute__((section(".data"))) systick64;

static DEFINE_SPINLOCK(lock_systick64);

uint64_t get_systick64()
{
	uint64_t stamp = 0;

#ifdef CONFIG_SMP
	do {
		if (!is_locked(lock_systick64))
#endif
			stamp = systick64;
#ifdef CONFIG_SMP
	} while (is_locked(lock_systick64));
#endif

	return stamp;
}

static inline void update_tick(unsigned int delta)
{
	unsigned int irqflag;

	spin_lock_irqsave(&lock_systick64, irqflag);
	systick64 += delta;
	spin_unlock_irqrestore(&lock_systick64, irqflag);
}

static int nticks_khz;

static void isr_systick()
{
#ifdef CONFIG_TIMER_MS
	static int ms;

	systick_ms++;

	if (++ms >= nticks_khz) {
		ms = 0;
		update_tick(1);
		resched();
	}
#else
	systick_ms += nticks_khz;
	update_tick(1);
	resched();
#endif
}

#include <kernel/init.h>
#include <asm/sysclk.h>

void __init systick_init()
{
	register_isr(sysclk_init(), isr_systick);

	nticks_khz = KHZ / sysfreq;

	run_sysclk();
}
