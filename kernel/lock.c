#include "kernel/lock.h"

void lock_init(struct lock * const lock)
{
	list_init(&lock->waitq);
	lock->ticket = lock->turn = 0;
}

void mutex_init(struct lock * const lock)
{
	lock_init(lock);
}
