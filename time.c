#include <time.h>
#include <foundation.h>

unsigned long volatile __attribute__((section(".data"))) ticks;
unsigned long long __attribute__((section(".data"))) ticks_64;

DEFINE_SPINLOCK(lock_ticks_64);

unsigned long long get_ticks_64()
{
	unsigned long long stamp;
	unsigned long irq_flag;

	spinlock_irqsave(&lock_ticks_64, &irq_flag);

	stamp = ticks_64;

	spinlock_irqrestore(&lock_ticks_64, &irq_flag);

	return stamp;
}

void update_curr(unsigned elapsed)
{
	spin_lock(&lock_ticks_64);
	preempt_disable();

	ticks_64 += elapsed;

	preempt_enable();
	spin_unlock(&lock_ticks_64);
}
