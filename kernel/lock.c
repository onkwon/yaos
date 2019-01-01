#include "kernel/lock.h"

void lock_init(struct lock * const lock)
{
	lock->ticket = 0;
	lock->turn = 0;
	list_init(&lock->waitq);
}
