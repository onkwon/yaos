#include <kernel/lock.h>

volatile unsigned int __attribute__((section(".data"))) jiffies;
uint64_t __attribute__((section(".data"))) jiffies_64;

static DEFINE_RWLOCK(lock_jiffies_64);

uint64_t get_jiffies_64_core()
{
	return jiffies_64;
}

uint64_t get_jiffies_64()
{
	uint64_t stamp = 0;

	read_lock(lock_jiffies_64);
	stamp = get_jiffies_64_core();
	read_unlock(lock_jiffies_64);

	return stamp;
}

void inline update_tick(unsigned delta)
{
	write_lock(lock_jiffies_64);
	jiffies_64 += delta;
	write_unlock(lock_jiffies_64);
}
