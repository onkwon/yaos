#include <time.h>
#include <foundation.h>

unsigned long volatile __attribute__((section(".data"))) systick;
uint64_t __attribute__((section(".data"))) systick_64;

DEFINE_SPINLOCK(lock_systick);

unsigned long long inline __get_systick_64()
{
	return systick_64;
}

unsigned long long get_systick_64()
{
	unsigned long long stamp;
	unsigned long irq_flag;

	spinlock_irqsave(lock_systick, irq_flag);

	stamp = __get_systick_64();

	spinlock_irqrestore(lock_systick, irq_flag);

	return stamp;
}

void inline update_tick(unsigned delta)
{
	/* In multi processor system, systick_64 is global meaning it is
	 * accessed by all processors while others related to a scheduler
	 * are accessed by only its processor. */

	preempt_disable();
	spin_lock(lock_systick);

	systick_64 += delta;

	spin_unlock(lock_systick);
	preempt_enable();
}
