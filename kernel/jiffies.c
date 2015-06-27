#include <kernel/lock.h>

volatile unsigned int __attribute__((section(".data"))) jiffies;
uint64_t __attribute__((section(".data"))) jiffies_64;

static DEFINE_SPINLOCK(lock_jiffies_64);

uint64_t get_jiffies_64()
{
	uint64_t stamp = 0;

#ifdef CONFIG_SMP
	do {
		if (!is_locked(lock_jiffies_64))
#endif
			stamp = jiffies_64;
#ifdef CONFIG_SMP
	} while (is_locked(lock_jiffies_64));
#endif

	return stamp;
}

void inline update_tick(unsigned delta)
{
	unsigned int irqflag;

	spin_lock_irqsave(lock_jiffies_64, irqflag);
	jiffies_64 += delta;
	spin_unlock_irqrestore(lock_jiffies_64, irqflag);
}
