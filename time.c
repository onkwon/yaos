#include "time.h"

unsigned long __attribute__((section(".data"))) ticks;
unsigned long long __attribute__((section(".data"))) ticks_64;

#include "foundation.h"

unsigned long long get_ticks_64()
{
	unsigned long long stamp;

	DEFINE_SPINLOCK(lock);
	unsigned long irq_flag;

	spinlock_irqsave(&lock, &irq_flag);
	stamp = ticks_64;
	spinlock_irqrestore(&lock, &irq_flag);

	return stamp;
}
