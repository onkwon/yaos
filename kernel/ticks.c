#include <kernel/lock.h>

volatile unsigned int __attribute__((section(".data"))) ticks;
uint64_t __attribute__((section(".data"))) ticks_64;

static DEFINE_SPINLOCK(lock_ticks_64);

uint64_t get_ticks_64()
{
	uint64_t stamp = 0;

#ifdef CONFIG_SMP
	do {
		if (!is_locked(lock_ticks_64))
#endif
			stamp = ticks_64;
#ifdef CONFIG_SMP
	} while (is_locked(lock_ticks_64));
#endif

	return stamp;
}

static inline void update_tick(unsigned int delta)
{
	unsigned int irqflag;

	spin_lock_irqsave(lock_ticks_64, irqflag);
	ticks_64 += delta;
	spin_unlock_irqrestore(lock_ticks_64, irqflag);
}

void isr_sysclk()
{
	update_tick(1);
	sys_schedule();
}

#include <foundation.h>
#include <kernel/init.h>

void __init sysclk_init()
{
	register_isr(NV_TICK, isr_sysclk);

	reset_sysclk();
	set_sysclk(get_sysclk_hz() / HZ - 1);
}
