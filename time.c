#include <time.h>
#include <foundation.h>

unsigned long volatile __attribute__((section(".data"))) jiffies;
uint64_t __attribute__((section(".data"))) jiffies_64;

DEFINE_SPINLOCK(lock_jiffies);

unsigned long long inline __get_jiffies_64()
{
	return jiffies_64;
}

unsigned long long get_jiffies_64()
{
	unsigned long long stamp;
	unsigned long irq_flag;

	spinlock_irqsave(lock_jiffies, irq_flag);

	stamp = __get_jiffies_64();

	spinlock_irqrestore(lock_jiffies, irq_flag);

	return stamp;
}

void inline update_tick(unsigned delta)
{
	/* In multi processor system, jiffies_64 is global, meaning it is
	 * accessed by all processors while others related to a scheduler
	 * are accessed by only its processor. */

	preempt_disable();
	spin_lock(lock_jiffies);

	jiffies_64 += delta;

	spin_unlock(lock_jiffies);
	preempt_enable();
}
